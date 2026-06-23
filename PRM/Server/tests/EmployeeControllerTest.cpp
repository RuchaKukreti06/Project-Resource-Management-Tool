#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>

#include "httplib.h"
#include "controllers/EmployeeController.h"
#include "middleware/AuthMiddleware.h"
#include "services/interfaces/ITokenService.h"
#include "services/interfaces/IEmployeeService.h"
#include "exceptions/Exceptions.h"
#include "utils/GlobalExceptionHandler.h"

class ConfigurableTokenServiceForEmployee : public ITokenService {
public:
    int userId = 1;
    std::string role = "ADMIN";

    std::string generateToken(const User&) const override { return ""; }
    bool validateToken(const std::string&) const override { return true; }
    int getClaimUserId(const std::string&) const override { return userId; }
    std::string getClaimRole(const std::string&) const override { return role; }
    bool getClaimForcePasswordChange(const std::string&) const override { return false; }
};

class DummyEmployeeServiceForController : public IEmployeeService {
public:
    void createEmployeeProfile(const EmployeeCreateRequest&) override {}
    void updateEmployeeProfile(const Employee&) override {}
    void deactivateEmployee(int, const std::string&) override {}
    std::vector<EmployeeResponse> getAllEmployees() override { return {}; }
    std::vector<EmployeeResponse> getTeamEmployees(int) override { return {}; }
    std::optional<Employee> getEmployeeById(int id) override { 
        Employee e; e.id = id; return e; 
    }
    std::optional<Employee> getEmployeeByUserId(int userId) override { 
        Employee e; e.user_id = userId; return e; 
    }
    void addSkill(const AddSkillRequest&) override {}
    void updateSkill(const UpdateSkillRequest&) override {}
    void removeSkill(int, int) override {}
    std::vector<EmployeeSkillResponse> getSkills(int) override { return {}; }
};

class EmployeeControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        configurableTokenService = std::make_shared<ConfigurableTokenServiceForEmployee>();
        dummyEmployeeService = std::make_shared<DummyEmployeeServiceForController>();
        
        authMiddleware = std::make_unique<AuthMiddleware>(*configurableTokenService);
        controller = std::make_unique<EmployeeController>(*dummyEmployeeService, *configurableTokenService);
        
        serverThread = std::thread([this]() {
            authMiddleware->registerMiddleware(server);
            controller->registerRoutes(server);
            utils::GlobalExceptionHandler::registerGlobalExceptionHandler(server);
            server.listen("localhost", 8090);
        });
        
        // Give server a moment to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void TearDown() override
    {
        server.stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    std::shared_ptr<ConfigurableTokenServiceForEmployee> configurableTokenService;
    std::shared_ptr<DummyEmployeeServiceForController> dummyEmployeeService;
    std::unique_ptr<AuthMiddleware> authMiddleware;
    std::unique_ptr<EmployeeController> controller;
    httplib::Server server;
    std::thread serverThread;
};

// 1. Admin can update any employee.
TEST_F(EmployeeControllerTest, UpdateEmployee_Admin_AllowsAccess)
{
    configurableTokenService->role = "ADMIN";
    httplib::Client cli("localhost", 8090);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"full_name": "Test"})";
    auto res = cli.Put("/employees/1", headers, body, "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
}

// 2. Manager cannot update employee profile.
TEST_F(EmployeeControllerTest, UpdateEmployee_Manager_DeniesAccess)
{
    configurableTokenService->role = "MANAGER";
    httplib::Client cli("localhost", 8090);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"full_name": "Test"})";
    auto res = cli.Put("/employees/1", headers, body, "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

// 3. Employee cannot update another employee profile.
TEST_F(EmployeeControllerTest, UpdateEmployee_Employee_DeniesAccess)
{
    configurableTokenService->role = "EMPLOYEE";
    httplib::Client cli("localhost", 8090);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"full_name": "Test"})";
    auto res = cli.Put("/employees/1", headers, body, "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

// 4. Admin can deactivate employee.
TEST_F(EmployeeControllerTest, DeactivateEmployee_Admin_AllowsAccess)
{
    configurableTokenService->role = "ADMIN";
    httplib::Client cli("localhost", 8090);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    auto res = cli.Patch("/employees/1/deactivate", headers, "", "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
}

// 5. Manager cannot deactivate employee.
TEST_F(EmployeeControllerTest, DeactivateEmployee_Manager_DeniesAccess)
{
    configurableTokenService->role = "MANAGER";
    httplib::Client cli("localhost", 8090);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    auto res = cli.Patch("/employees/1/deactivate", headers, "", "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

// 6. Employee cannot deactivate employee.
TEST_F(EmployeeControllerTest, DeactivateEmployee_Employee_DeniesAccess)
{
    configurableTokenService->role = "EMPLOYEE";
    httplib::Client cli("localhost", 8090);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    auto res = cli.Patch("/employees/1/deactivate", headers, "", "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

// 7. Admin can add/update/remove skills.
TEST_F(EmployeeControllerTest, AddSkill_Admin_AllowsAccess)
{
    configurableTokenService->role = "ADMIN";
    httplib::Client cli("localhost", 8090);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"skill_name": "C++", "category": "Tech", "proficiency": "Expert"})";
    auto res = cli.Post("/employees/1/skills", headers, body, "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 201);
}

// 8. Manager cannot add/update/remove skills.
TEST_F(EmployeeControllerTest, AddSkill_Manager_DeniesAccess)
{
    configurableTokenService->role = "MANAGER";
    httplib::Client cli("localhost", 8090);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"skill_name": "C++", "category": "Tech", "proficiency": "Expert"})";
    auto res = cli.Post("/employees/1/skills", headers, body, "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

// 9. Employee cannot add/update/remove skills.
TEST_F(EmployeeControllerTest, AddSkill_Employee_DeniesAccess)
{
    configurableTokenService->role = "EMPLOYEE";
    httplib::Client cli("localhost", 8090);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"skill_name": "C++", "category": "Tech", "proficiency": "Expert"})";
    auto res = cli.Post("/employees/1/skills", headers, body, "application/json");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}
