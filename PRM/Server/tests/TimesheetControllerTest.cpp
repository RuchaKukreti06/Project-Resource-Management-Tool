#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>

#include "httplib.h"
#include "controllers/TimesheetController.h"
#include "mocks/MockTimesheetService.h"
#include "mocks/MockNotificationService.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Throw;
#include "exceptions/Exceptions.h"
#include "utils/GlobalExceptionHandler.h"

class TimesheetControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockTimesheetService = std::make_shared<MockTimesheetService>();
        mockNotificationService = std::make_shared<MockNotificationService>();
        controller = std::make_unique<TimesheetController>(*mockTimesheetService, *mockNotificationService);
        
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
    std::unique_ptr<TimesheetController> controller;
    httplib::Server server;
    std::thread serverThread;
};

TEST_F(TimesheetControllerTest, RestoreTimesheetAccess_Success)
{
    EXPECT_CALL(*mockNotificationService, restoreTimesheetAccess(123, "2024-01-01"));

    httplib::Client cli("localhost", 8089);
    std::string body = R"({"user_id": 123, "week_start_date": "2024-01-01"})";
    
    auto res = cli.Put("/timesheets/access/restore", body, "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"success\":true"), std::string::npos);
    EXPECT_NE(res->body.find("\"message\":\"Timesheet access restored.\""), std::string::npos);
}

TEST_F(TimesheetControllerTest, RestoreTimesheetAccess_Failure)
{
    EXPECT_CALL(*mockNotificationService, restoreTimesheetAccess(123, "2024-01-01"))
        .WillOnce(Throw(exceptions::ValidationException("Failed to restore.")));

    httplib::Client cli("localhost", 8089);
    std::string body = R"({"user_id": 123, "week_start_date": "2024-01-01"})";
    
    auto res = cli.Put("/timesheets/access/restore", body, "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400);
    EXPECT_NE(res->body.find("\"success\":false"), std::string::npos);
    EXPECT_NE(res->body.find("\"error\":\"Failed to restore.\""), std::string::npos);
}
