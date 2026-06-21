#include <gtest/gtest.h>

#include <memory>
#include <thread>
#include <chrono>

#include "AuthController.h"
#include "AuthService.h"
#include "AuthConfig.h"
#include "ConfigLoader.h"
#include "MockUserRepository.h"
#include "services/PasswordHasher.h"
#include "services/JwtTokenService.h"
#include "exceptions/Exceptions.h"
#include "utils/GlobalExceptionHandler.h"

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
        auto hasher = std::make_shared<PasswordHasher>();
        auto tokenService = std::make_shared<JwtTokenService>(AuthConfig{"test-jwt-secret", 60});

        authService = std::make_unique<AuthService>(
            repo, hasher, tokenService);

        controller = std::make_unique<AuthController>(*authService);

        serverThread = std::thread([this]() {
            controller->registerRoutes(server);
            utils::GlobalExceptionHandler::registerGlobalExceptionHandler(server);
            server.listen("localhost", 8090);
        });
        
        // Wait for server to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void TearDown() override
    {
        server.stop();
        if (serverThread.joinable())
        {
            serverThread.join();
        }
    }

    httplib::Server server;
    std::thread serverThread;

    std::shared_ptr<MockUserRepository> repo;
    std::unique_ptr<AuthService> authService;
    std::unique_ptr<AuthController> controller;
};

TEST_F(AuthControllerTest, RegisterSuccess)
{
    httplib::Client cli("localhost", 8090);
    std::string body = R"({
        "username":"alice",
        "password":"Password123",
        "email":"alice@example.com",
        "full_name":"Alice Smith"
    })";

    auto res = cli.Post("/auth/register", body, "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    auto json = nlohmann::json::parse(res->body);
    EXPECT_TRUE(json["success"].get<bool>());
}

TEST_F(AuthControllerTest, RegisterDuplicateUser)
{
    httplib::Client cli("localhost", 8090);
    std::string body = R"({
        "username":"alice",
        "password":"Password123",
        "email":"alice@example.com",
        "full_name":"Alice Smith"
    })";

    auto res1 = cli.Post("/auth/register", body, "application/json");
    ASSERT_TRUE(res1);
    EXPECT_EQ(res1->status, 200);

    auto res2 = cli.Post("/auth/register", body, "application/json");
    ASSERT_TRUE(res2);
    EXPECT_EQ(res2->status, 409);

    auto json = nlohmann::json::parse(res2->body);
    EXPECT_FALSE(json["success"].get<bool>());
    EXPECT_EQ(json["code"].get<int>(), 409);
    EXPECT_NE(json["error"].get<std::string>(), "");
}

TEST_F(AuthControllerTest, RegisterMissingUsername)
{
    httplib::Client cli("localhost", 8090);
    std::string body = R"({
        "password":"Password123",
        "email":"alice@example.com",
        "full_name":"Alice Smith"
    })";

    auto res = cli.Post("/auth/register", body, "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400);

    auto json = nlohmann::json::parse(res->body);
    EXPECT_FALSE(json["success"].get<bool>());
    EXPECT_EQ(json["code"].get<int>(), 400);
}

TEST_F(AuthControllerTest, LoginSuccess)
{
    authService->registerUser({"bob", "Password123", "bob@example.com", "Bob Jones"});

    httplib::Client cli("localhost", 8090);
    std::string body = R"({
        "username":"bob",
        "password":"Password123"
    })";

    auto res = cli.Post("/auth/login", body, "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    auto json = nlohmann::json::parse(res->body);
    EXPECT_TRUE(json["success"].get<bool>());
    EXPECT_TRUE(json.contains("token"));
    
    // Verify generated JWT
    std::string token = json["token"].get<std::string>();
    EXPECT_FALSE(token.empty());
    
    JwtTokenService tokenService(AuthConfig{"test-jwt-secret", 60});
    EXPECT_TRUE(tokenService.validateToken(token));
}

TEST_F(AuthControllerTest, LoginWrongPassword)
{
    authService->registerUser({"bob", "Password123", "bob@example.com", "Bob Jones"});

    httplib::Client cli("localhost", 8090);
    std::string body = R"({
        "username":"bob",
        "password":"WrongPassword"
    })";

    auto res = cli.Post("/auth/login", body, "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 401);

    auto json = nlohmann::json::parse(res->body);
    EXPECT_FALSE(json["success"].get<bool>());
    EXPECT_EQ(json["code"].get<int>(), 401);
}

TEST_F(AuthControllerTest, LoginInvalidJson)
{
    httplib::Client cli("localhost", 8090);
    std::string body = "{invalid json";

    auto res = cli.Post("/auth/login", body, "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400);

    auto json = nlohmann::json::parse(res->body);
    EXPECT_FALSE(json["success"].get<bool>());
    EXPECT_EQ(json["code"].get<int>(), 400);
}

TEST_F(AuthControllerTest, ChangePasswordSuccess)
{
    authService->registerUser({"john", "OldPassword", "john@example.com", "John Doe"});

    User user = repo->getUserByUsername("john");

    httplib::Client cli("localhost", 8090);
    std::string body = nlohmann::json{{"userId", user.id}, {"newPassword", "NewPassword"}}.dump();

    auto res = cli.Post("/auth/change-password", body, "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    auto json = nlohmann::json::parse(res->body);
    EXPECT_TRUE(json["success"].get<bool>());
}

TEST_F(AuthControllerTest, ChangePasswordUnknownUser)
{
    httplib::Client cli("localhost", 8090);
    std::string body = nlohmann::json{{"userId", 9999}, {"newPassword", "NewPassword"}}.dump();

    auto res = cli.Post("/auth/change-password", body, "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 404);

    auto json = nlohmann::json::parse(res->body);
    EXPECT_FALSE(json["success"].get<bool>());
    EXPECT_EQ(json["code"].get<int>(), 404);
}

TEST_F(AuthControllerTest, LoginDisabledUser)
{
    authService->registerUser({"disabled", "Password123", "disabled@example.com", "Disabled User"});
    User user = repo->getUserByUsername("disabled");
    repo->setUserStatus(user.id, "INACTIVE");

    httplib::Client cli("localhost", 8090);
    std::string body = R"({
        "username":"disabled",
        "password":"Password123"
    })";

    auto res = cli.Post("/auth/login", body, "application/json");

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 401);

    auto json = nlohmann::json::parse(res->body);
    EXPECT_FALSE(json["success"].get<bool>());
    EXPECT_EQ(json["code"].get<int>(), 401);
}

TEST_F(AuthControllerTest, TokenValidation)
{
    authService->registerUser({"charlie", "Password123", "charlie@example.com", "Charlie"});

    httplib::Client cli("localhost", 8090);
    std::string body = R"({
        "username":"charlie",
        "password":"Password123"
    })";

    auto res = cli.Post("/auth/login", body, "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    auto json = nlohmann::json::parse(res->body);
    std::string token = json["token"].get<std::string>();

    EXPECT_TRUE(authService->validateToken(token));
    EXPECT_FALSE(authService->validateToken("invalid.token.string"));
}
