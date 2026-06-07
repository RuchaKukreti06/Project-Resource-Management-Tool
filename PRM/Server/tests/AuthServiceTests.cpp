#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "AuthService.h"
#include "ConfigLoader.h"
#include "IUserRepository.h"
#include "User.h"

class MockUserRepository : public IUserRepository
{
   public:
    MockUserRepository() = default;

    bool createUser(const User& user) override
    {
        if (usernameIndex_.find(user.username) != usernameIndex_.end())
        {
            return false;
        }
        User stored = user;
        stored.id = nextId_++;
        usersById_[stored.id] = stored;
        usernameIndex_[stored.username] = stored.id;
        return true;
    }

    User getUserById(int id) override
    {
        auto it = usersById_.find(id);
        return it == usersById_.end() ? User{} : it->second;
    }

    User getUserByUsername(const std::string& username) override
    {
        auto it = usernameIndex_.find(username);
        if (it == usernameIndex_.end()) return User{};
        return usersById_[it->second];
    }

    std::vector<User> getAllUsers() override
    {
        std::vector<User> all;
        all.reserve(usersById_.size());
        for (auto& pair : usersById_) all.push_back(pair.second);
        return all;
    }

    bool updateUser(const User& user) override
    {
        auto it = usersById_.find(user.id);
        if (it == usersById_.end()) return false;
        usersById_[user.id] = user;
        usernameIndex_[user.username] = user.id;
        return true;
    }

    bool deleteUser(int id) override
    {
        auto it = usersById_.find(id);
        if (it == usersById_.end()) return false;
        usernameIndex_.erase(it->second.username);
        usersById_.erase(it);
        return true;
    }

    bool updatePassword(int id, const std::string& passwordHash) override
    {
        auto it = usersById_.find(id);
        if (it == usersById_.end()) return false;
        it->second.passwordHash = passwordHash;
        return true;
    }

   private:
    std::unordered_map<int, User> usersById_;
    std::unordered_map<std::string, int> usernameIndex_;
    int nextId_ = 1;
};

static std::filesystem::path getTestConfigPath()
{
    return std::filesystem::path(__FILE__).parent_path() / "config" / "test_config.json";
}

static void loadTestConfiguration()
{
    ASSERT_NO_THROW({ utils::ConfigLoader::instance().load(getTestConfigPath().string()); })
        << "Configuration load failed.";
}

class AuthServiceTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        utils::ConfigLoader::instance().load(getTestConfigPath().string());

        repo_ = std::make_shared<MockUserRepository>();
        auth_ = std::make_unique<AuthService>(repo_);
    }

    std::shared_ptr<MockUserRepository> repo_;
    std::unique_ptr<AuthService> auth_;
};

TEST_F(AuthServiceTest, RegisterNewUser_Succeeds)
{
    auto response = auth_->registerUser("alice", "Password123");
    EXPECT_TRUE(response["success"].get<bool>());
    EXPECT_EQ(response["message"].get<std::string>(), "Registration successful.");
}

TEST_F(AuthServiceTest, RegisterDuplicateUser_Fails)
{
    auth_->registerUser("alice", "Password123");
    auto duplicate = auth_->registerUser("alice", "Password123");
    EXPECT_FALSE(duplicate["success"].get<bool>());
    EXPECT_EQ(duplicate["message"].get<std::string>(), "Username already exists.");
}

// ─────────────────────────────────────────────
//  Tests — Login
// ─────────────────────────────────────────────

TEST_F(AuthServiceTest, Login_UnknownUser_Fails)
{
    auto result = auth_->login("nobody", "Secret1");
    EXPECT_FALSE(result["success"].get<bool>());
    EXPECT_EQ(result["message"].get<std::string>(), "Invalid username or password.");
}

TEST_F(AuthServiceTest, Login_WrongPassword_Fails)
{
    auth_->registerUser("bob", "Secret1");
    auto result = auth_->login("bob", "WrongSecret");
    EXPECT_FALSE(result["success"].get<bool>());
    EXPECT_EQ(result["message"].get<std::string>(), "Invalid username or password.");
}

TEST_F(AuthServiceTest, Login_InactiveAccount_Fails)
{
    auth_->registerUser("carol", "Secret1");

    User inactive = repo_->getUserByUsername("carol");
    inactive.status = "inactive";
    repo_->updateUser(inactive);

    auto result = auth_->login("carol", "Secret1");
    EXPECT_FALSE(result["success"].get<bool>());
    EXPECT_EQ(result["message"].get<std::string>(), "Account is not active.");
}

TEST_F(AuthServiceTest, ChangePassword_UpdatesStoredHash)
{
    auth_->registerUser("dave", "Password1");
    User user = repo_->getUserByUsername("dave");
    ASSERT_NE(user.id, 0) << "Created user must have a valid ID.";

    const std::string oldHash = user.passwordHash;
    EXPECT_TRUE(auth_->changePassword(user.id, "NewPassword2"));

    User updated = repo_->getUserById(user.id);
    EXPECT_NE(updated.passwordHash, oldHash);
}

TEST_F(AuthServiceTest, ChangePassword_NewPasswordAllowsLogin)
{
    auth_->registerUser("dave", "Password1");
    User user = repo_->getUserByUsername("dave");
    auth_->changePassword(user.id, "NewPassword2");

    auto result = auth_->login("dave", "NewPassword2");
    ASSERT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["user"]["username"].get<std::string>(), "dave");
}

class AuthServiceTokenTest : public AuthServiceTest
{
   protected:
    void SetUp() override
    {
        AuthServiceTest::SetUp();
        utils::ConfigLoader::instance().load(getTestConfigPath().string());

        auth_->registerUser("eve", "Password1");
        user_ = repo_->getUserByUsername("eve");

        auto loginResult = auth_->login("eve", "Password1");
        token_ = loginResult["token"].get<std::string>();
    }

    User user_;
    std::string token_;
};

TEST_F(AuthServiceTokenTest, Login_ProducesNonEmptyToken)
{
    EXPECT_FALSE(token_.empty());
}

TEST_F(AuthServiceTokenTest, ValidateToken_AcceptsValidJWT)
{
    EXPECT_TRUE(auth_->validateToken(token_));
}

TEST_F(AuthServiceTokenTest, IsLoggedIn_TrueForValidToken)
{
    auth_->setToken(token_);
    EXPECT_TRUE(auth_->isLoggedIn());
}

TEST_F(AuthServiceTokenTest, ValidateToken_RejectsMalformedJWT)
{
    EXPECT_FALSE(auth_->validateToken("invalid-token"));
}

TEST_F(AuthServiceTokenTest, IsLoggedIn_FalseForInvalidToken)
{
    auth_->setToken("invalid-token");
    EXPECT_FALSE(auth_->isLoggedIn());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}