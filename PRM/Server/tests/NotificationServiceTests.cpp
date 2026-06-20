#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>

#include "services/NotificationService.h"
#include "mocks/MockNotificationRepository.h"
#include "mocks/MockUserRepository.h"

using ::testing::_;
using ::testing::Return;

class NotificationServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockRepo = std::make_shared<MockNotificationRepository>();
        mockUserRepo = std::make_shared<MockUserRepository>();
        // emailService is not needed for restoreTimesheetAccess tests, pass nullptr
        service = std::make_unique<NotificationService>(mockRepo, mockUserRepo, nullptr);
    }

    std::shared_ptr<MockNotificationRepository> mockRepo;
    std::shared_ptr<MockUserRepository> mockUserRepo;
    std::unique_ptr<NotificationService> service;
};

TEST_F(NotificationServiceTests, RestoreTimesheetAccess_InvalidInput_ReturnsFalse)
{
    std::string message;
    bool result = service->restoreTimesheetAccess(-1, "2024-01-01", message);
    EXPECT_FALSE(result);
    EXPECT_EQ(message, "user_id and week_start_date are required.");
}

TEST_F(NotificationServiceTests, RestoreTimesheetAccess_RepositoryFails_ReturnsFalse)
{
    EXPECT_CALL(*mockRepo, restoreTimesheetAccess(1, "2024-01-01"))
        .WillOnce(Return(false));

    std::string message;
    bool result = service->restoreTimesheetAccess(1, "2024-01-01", message);
    EXPECT_FALSE(result);
    EXPECT_EQ(message, "Failed to restore timesheet access.");
}

TEST_F(NotificationServiceTests, RestoreTimesheetAccess_RepositorySucceeds_ReturnsTrue)
{
    EXPECT_CALL(*mockRepo, restoreTimesheetAccess(1, "2024-01-01"))
        .WillOnce(Return(true));

    std::string message;
    bool result = service->restoreTimesheetAccess(1, "2024-01-01", message);
    EXPECT_TRUE(result);
    EXPECT_EQ(message, "Timesheet access restored.");
}
