#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "AuthService.h"
#include "AuthConfig.h"
#include "IUserRepository.h"
#include "MockUserRepository.h"
#include "services/PasswordHasher.h"
#include "services/JwtTokenService.h"
#include "User.h"


class AuthServiceTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        repo_ = std::make_shared<MockUserRepository>();
        auto hasher = std::make_shared<PasswordHasher>();
        auto tokenService = std::make_shared<JwtTokenService>(AuthConfig{"test-jwt-secret", 60});
        auth_ = std::make_unique<AuthService>(
            repo_, hasher, tokenService);
    }

    std::shared_ptr<MockUserRepository> repo_;
    std::unique_ptr<AuthService> auth_;
};

TEST_F(AuthServiceTest, RegisterNewUser_Succeeds)
{
    auto response = auth_->registerUser({"alice", "Password123", "alice@example.com", "Alice Smith"});
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.message, "Registration successful.");
}

TEST_F(AuthServiceTest, RegisterDuplicateUser_Fails)
{
    auth_->registerUser({"alice", "Password123", "alice@example.com", "Alice Smith"});
    auto duplicate = auth_->registerUser({"alice", "Password123", "alice2@example.com", "Alice Smith"});
    EXPECT_FALSE(duplicate.success);
    EXPECT_EQ(duplicate.message, "Username already exists.");
}

// ─────────────────────────────────────────────
//  Tests — Login
// ─────────────────────────────────────────────

TEST_F(AuthServiceTest, Login_UnknownUser_Fails)
{
    LoginRequest req; req.username = "nobody"; req.password = "Secret1";
    auto result = auth_->login(req);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.message, "Invalid username or password.");
}

TEST_F(AuthServiceTest, Login_WrongPassword_Fails)
{
    auth_->registerUser({"bob", "Secret1", "bob@example.com", "Bob Jones"});
    LoginRequest req; req.username = "bob"; req.password = "WrongSecret";
    auto result = auth_->login(req);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.message, "Invalid username or password.");
}

TEST_F(AuthServiceTest, Login_InactiveAccount_Fails)
{
    auth_->registerUser({"carol", "Secret1", "carol@example.com", "Carol White"});

    User inactive = repo_->getUserByUsername("carol");
    inactive.status = "INACTIVE";
    repo_->updateUser(inactive);

    LoginRequest req; req.username = "carol"; req.password = "Secret1";
    auto result = auth_->login(req);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.message, "Account is not active.");
}

TEST_F(AuthServiceTest, ChangePassword_UpdatesStoredHash)
{
    auth_->registerUser({"dave", "Password1", "dave@example.com", "Dave Brown"});
    User user = repo_->getUserByUsername("dave");
    ASSERT_NE(user.id, 0) << "Created user must have a valid ID.";

    const std::string oldHash = user.passwordHash;
    EXPECT_TRUE(auth_->changePassword({user.id, "NewPassword2"}));

    User updated = repo_->getUserById(user.id);
    EXPECT_NE(updated.passwordHash, oldHash);
}

TEST_F(AuthServiceTest, ChangePassword_NewPasswordAllowsLogin)
{
    auth_->registerUser({"dave", "Password1", "dave@example.com", "Dave Brown"});
    User user = repo_->getUserByUsername("dave");
    auth_->changePassword({user.id, "NewPassword2"});

    LoginRequest req; req.username = "dave"; req.password = "NewPassword2";
    auto result = auth_->login(req);
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.username, "dave");
}

class AuthServiceTokenTest : public AuthServiceTest
{
   protected:
    void SetUp() override
    {
        AuthServiceTest::SetUp();

        auth_->registerUser({"eve", "Password1", "eve@example.com", "Eve Green"});
        user_ = repo_->getUserByUsername("eve");

        LoginRequest req; req.username = "eve"; req.password = "Password1";
        auto loginResult = auth_->login(req);
        token_ = loginResult.token;
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