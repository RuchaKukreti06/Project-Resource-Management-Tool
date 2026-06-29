#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>

#include "httplib.h"
#include "controllers/TimesheetController.h"
#include "mocks/MockTimesheetService.h"
#include "mocks/MockNotificationService.h"
#include "services/interfaces/ITokenService.h"
#include "services/interfaces/IEmployeeService.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Throw;
#include "exceptions/Exceptions.h"
#include "utils/GlobalExceptionHandler.h"

class ConfigurableTokenService : public ITokenService {
public:
    int userId = 1;
    std::string role = "ADMIN";

    std::string generateToken(const User&) const override { return ""; }
    bool validateToken(const std::string&) const override { return true; }
    int getClaimUserId(const std::string&) const override { return userId; }
    std::string getClaimRole(const std::string&) const override { return role; }
    bool getClaimForcePasswordChange(const std::string&) const override { return false; }
};

class ConfigurableEmployeeService : public IEmployeeService {
public:
    Employee returnEmployee;

    void createEmployeeProfile(const EmployeeCreateRequest&) override {}
    void updateEmployeeProfile(const Employee&) override {}
    void deactivateEmployee(int, const std::string&) override {}
    std::vector<EmployeeResponse> getAllEmployees() override { return {}; }
    std::vector<EmployeeResponse> getTeamEmployees(int) override { return {}; }
    std::optional<Employee> getEmployeeById(int id) override { 
        returnEmployee.id = id;
        return returnEmployee; 
    }
    std::optional<Employee> getEmployeeByUserId(int userId) override { 
        returnEmployee.user_id = userId;
        return returnEmployee; 
    }
    void addSkill(const AddSkillRequest&) override {}
    void updateSkill(const UpdateSkillRequest&) override {}
    void removeSkill(int, int) override {}
    std::vector<EmployeeSkillResponse> getSkills(int) override { return {}; }
};

class TimesheetControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockTimesheetService = std::make_shared<MockTimesheetService>();
        mockNotificationService = std::make_shared<MockNotificationService>();
        configurableTokenService = std::make_shared<ConfigurableTokenService>();
        configurableEmployeeService = std::make_shared<ConfigurableEmployeeService>();
        controller = std::make_unique<TimesheetController>(*mockTimesheetService, *mockNotificationService, *configurableTokenService, *configurableEmployeeService);
        
        serverThread = std::thread([this]() {
            controller->registerRoutes(server);
            utils::GlobalExceptionHandler::registerGlobalExceptionHandler(server);
            server.listen("localhost", 8089);
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

    std::shared_ptr<MockTimesheetService> mockTimesheetService;
    std::shared_ptr<MockNotificationService> mockNotificationService;
    std::shared_ptr<ConfigurableTokenService> configurableTokenService;
    std::shared_ptr<ConfigurableEmployeeService> configurableEmployeeService;
    std::unique_ptr<TimesheetController> controller;
    httplib::Server server;
    std::thread serverThread;
};

TEST_F(TimesheetControllerTest, RestoreTimesheetAccess_Admin_DeniesAccess)
{
    configurableTokenService->role = "ADMIN";
    configurableTokenService->userId = 999;
    configurableEmployeeService->returnEmployee.user_id = 123;
    configurableEmployeeService->returnEmployee.manager_id = 222;

    httplib::Client cli("localhost", 8089);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"user_id": 123, "week_start_date": "2024-01-01"})";
    
    auto res = cli.Put("/timesheets/access/restore", headers, body, "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

TEST_F(TimesheetControllerTest, RestoreTimesheetAccess_Manager_ServiceFailure_Returns400)
{
    configurableTokenService->role = "MANAGER";
    configurableTokenService->userId = 222;
    configurableEmployeeService->returnEmployee.user_id = 123;
    configurableEmployeeService->returnEmployee.manager_id = 222;

    EXPECT_CALL(*mockNotificationService, restoreTimesheetAccess(123, "2024-01-01"))
        .WillOnce(Throw(exceptions::ValidationException("Failed to restore.")));

    httplib::Client cli("localhost", 8089);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"user_id": 123, "week_start_date": "2024-01-01"})";
    
    auto res = cli.Put("/timesheets/access/restore", headers, body, "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400);
    EXPECT_NE(res->body.find("\"success\":false"), std::string::npos);
    EXPECT_NE(res->body.find("\"error\":\"Failed to restore.\""), std::string::npos);
}

TEST_F(TimesheetControllerTest, RestoreTimesheetAccess_ManagerA_EmployeeA_AllowsAccess)
{
    configurableTokenService->role = "MANAGER";
    configurableTokenService->userId = 222;
    configurableEmployeeService->returnEmployee.user_id = 123;
    configurableEmployeeService->returnEmployee.manager_id = 222; // Same as tokenUserId

    EXPECT_CALL(*mockNotificationService, restoreTimesheetAccess(123, "2024-01-01"));

    httplib::Client cli("localhost", 8089);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"user_id": 123, "week_start_date": "2024-01-01"})";
    
    auto res = cli.Put("/timesheets/access/restore", headers, body, "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
}

TEST_F(TimesheetControllerTest, RestoreTimesheetAccess_ManagerA_EmployeeB_DeniesAccess)
{
    configurableTokenService->role = "MANAGER";
    configurableTokenService->userId = 222;
    configurableEmployeeService->returnEmployee.user_id = 123;
    configurableEmployeeService->returnEmployee.manager_id = 333; // Different manager

    httplib::Client cli("localhost", 8089);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"user_id": 123, "week_start_date": "2024-01-01"})";
    
    auto res = cli.Put("/timesheets/access/restore", headers, body, "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

TEST_F(TimesheetControllerTest, RestoreTimesheetAccess_Employee_DeniesAccess)
{
    configurableTokenService->role = "EMPLOYEE";
    configurableTokenService->userId = 123;
    configurableEmployeeService->returnEmployee.user_id = 123;
    configurableEmployeeService->returnEmployee.manager_id = 333; // Employee is not their own manager

    httplib::Client cli("localhost", 8089);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    std::string body = R"({"user_id": 123, "week_start_date": "2024-01-01"})";
    
    auto res = cli.Put("/timesheets/access/restore", headers, body, "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

TEST_F(TimesheetControllerTest, GetEmployeeTimesheets_Admin_DeniesAccess)
{
    configurableTokenService->role = "ADMIN";
    configurableTokenService->userId = 999;
    configurableEmployeeService->returnEmployee.user_id = 111;
    configurableEmployeeService->returnEmployee.manager_id = 222;

    httplib::Client cli("localhost", 8089);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    auto res = cli.Get("/employees/1/timesheets", headers);
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

TEST_F(TimesheetControllerTest, GetEmployeeTimesheets_ManagerA_ViewsEmployeeA_AllowsAccess)
{
    configurableTokenService->role = "MANAGER";
    configurableTokenService->userId = 222;
    configurableEmployeeService->returnEmployee.user_id = 111;
    configurableEmployeeService->returnEmployee.manager_id = 222; // Same as tokenUserId

    httplib::Client cli("localhost", 8089);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    auto res = cli.Get("/employees/1/timesheets", headers);
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
}

TEST_F(TimesheetControllerTest, GetEmployeeTimesheets_ManagerA_ViewsEmployeeB_DeniesAccess)
{
    configurableTokenService->role = "MANAGER";
    configurableTokenService->userId = 222;
    configurableEmployeeService->returnEmployee.user_id = 111;
    configurableEmployeeService->returnEmployee.manager_id = 333; // Different manager

    httplib::Client cli("localhost", 8089);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    auto res = cli.Get("/employees/1/timesheets", headers);
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}

TEST_F(TimesheetControllerTest, GetEmployeeTimesheets_EmployeeA_ViewsEmployeeB_DeniesAccess)
{
    configurableTokenService->role = "EMPLOYEE";
    configurableTokenService->userId = 111;
    configurableEmployeeService->returnEmployee.user_id = 222; // Different user
    configurableEmployeeService->returnEmployee.manager_id = 333;

    httplib::Client cli("localhost", 8089);
    httplib::Headers headers = {{"Authorization", "Bearer token"}};
    auto res = cli.Get("/employees/1/timesheets", headers);
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 403);
}
