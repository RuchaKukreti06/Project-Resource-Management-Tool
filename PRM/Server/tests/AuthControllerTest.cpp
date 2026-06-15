#include <gtest/gtest.h>

#include <memory>

#include "AuthController.h"
#include "AuthService.h"
#include "ConfigLoader.h"
#include "MockUserRepository.h"

static std::filesystem::path getTestConfigPath()
{
    return std::filesystem::path(__FILE__).parent_path() / "config" / "test_config.json";
}

static void loadTestConfiguration()
{
    ASSERT_NO_THROW({ utils::ConfigLoader::instance().load(getTestConfigPath().string()); })
        << "Configuration load failed.";
}

class AuthControllerTest : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        utils::ConfigLoader::instance().load(getTestConfigPath().string());
        repo = std::make_shared<MockUserRepository>();

        authService = std::make_unique<AuthService>(repo);

        controller = std::make_unique<AuthController>(*authService);
    }

    httplib::Request req;
    httplib::Response res;

    std::shared_ptr<MockUserRepository> repo;

    std::unique_ptr<AuthService> authService;

    std::unique_ptr<AuthController> controller;
};

TEST_F(AuthControllerTest, RegisterSuccess)
{
    req.body =
        R"({
            "username":"alice",
            "password":"Password123",
            "email":"alice@example.com",
            "full_name":"Alice Smith"
        })";

    controller->handleRegister(req, res);

    auto json = nlohmann::json::parse(res.body);

    EXPECT_EQ(res.status, 200);

    EXPECT_TRUE(json["success"].get<bool>());
}

TEST_F(AuthControllerTest, RegisterDuplicateUser)
{
    req.body =
        R"({
            "username":"alice",
            "password":"Password123",
            "email":"alice@example.com",
            "full_name":"Alice Smith"
        })";

    controller->handleRegister(req, res);

    httplib::Response secondRes;

    controller->handleRegister(req, secondRes);

    auto json = nlohmann::json::parse(secondRes.body);

    EXPECT_FALSE(json["success"].get<bool>());

    EXPECT_EQ(json["message"], "Username already exists.");
}

TEST_F(AuthControllerTest, RegisterMissingUsername)
{
    req.body =
        R"({
            "password":"Password123",
            "email":"alice@example.com",
            "full_name":"Alice Smith"
        })";

    controller->handleRegister(req, res);

    EXPECT_EQ(res.status, 400);

    auto json = nlohmann::json::parse(res.body);

    EXPECT_FALSE(json["success"].get<bool>());
}

TEST_F(AuthControllerTest, LoginSuccess)
{
    authService->registerUser("bob", "Password123", "bob@example.com", "Bob Jones");

    req.body =
        R"({
            "username":"bob",
            "password":"Password123"
        })";

    controller->handleLogin(req, res);

    auto json = nlohmann::json::parse(res.body);

    EXPECT_EQ(res.status, 200);

    EXPECT_TRUE(json["success"].get<bool>());

    EXPECT_TRUE(json.contains("token"));
}

TEST_F(AuthControllerTest, LoginWrongPassword)
{
    authService->registerUser("bob", "Password123", "bob@example.com", "Bob Jones");

    req.body =
        R"({
            "username":"bob",
            "password":"WrongPassword"
        })";

    controller->handleLogin(req, res);

    auto json = nlohmann::json::parse(res.body);

    EXPECT_FALSE(json["success"].get<bool>());

    EXPECT_EQ(json["message"], "Invalid username or password.");
}

TEST_F(AuthControllerTest, LoginInvalidJson)
{
    req.body = "{invalid json";

    controller->handleLogin(req, res);

    EXPECT_EQ(res.status, 400);

    auto json = nlohmann::json::parse(res.body);

    EXPECT_FALSE(json["success"].get<bool>());
}

TEST_F(AuthControllerTest, ChangePasswordSuccess)
{
    authService->registerUser("john", "OldPassword", "john@example.com", "John Doe");

    User user = repo->getUserByUsername("john");

    req.body = nlohmann::json{{"userId", user.id}, {"newPassword", "NewPassword"}}.dump();

    controller->handleChangePassword(req, res);

    auto json = nlohmann::json::parse(res.body);

    EXPECT_EQ(res.status, 200);

    EXPECT_TRUE(json["success"].get<bool>());
}

TEST_F(AuthControllerTest, ChangePasswordUnknownUser)
{
    req.body = nlohmann::json{{"userId", 9999}, {"newPassword", "NewPassword"}}.dump();

    controller->handleChangePassword(req, res);

    auto json = nlohmann::json::parse(res.body);

    EXPECT_EQ(res.status, 500);

    EXPECT_FALSE(json["success"].get<bool>());
}
