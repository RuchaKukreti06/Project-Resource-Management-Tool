#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>

#include "services/NotificationService.h"
#include "mocks/MockNotificationRepository.h"
#include "mocks/MockUserRepository.h"
#include "exceptions/Exceptions.h"

using ::testing::_;
using ::testing::Return;

class NotificationServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockRepo = std::make_shared<MockNotificationRepository>();
        mockUserRepo = std::make_shared<MockUserRepository>();
        // emailService and others are not needed for restoreTimesheetAccess tests, pass nullptr
        service = std::make_unique<NotificationService>(mockRepo, mockUserRepo, nullptr, nullptr, nullptr, nullptr);
    }

    std::shared_ptr<MockNotificationRepository> mockRepo;
    std::shared_ptr<MockUserRepository> mockUserRepo;
    std::unique_ptr<NotificationService> service;
};

TEST_F(NotificationServiceTests, RestoreTimesheetAccess_InvalidInput_ReturnsFalse)
{
    EXPECT_THROW({
        service->restoreTimesheetAccess(-1, "2024-01-01");
    }, exceptions::ValidationException);
}

TEST_F(NotificationServiceTests, RestoreTimesheetAccess_RepositoryFails_ReturnsFalse)
{
    EXPECT_CALL(*mockRepo, restoreTimesheetAccess(1, "2024-01-01"))
        .WillOnce(Return(false));

    EXPECT_THROW({
        service->restoreTimesheetAccess(1, "2024-01-01");
    }, exceptions::DatabaseException);
}

TEST_F(NotificationServiceTests, RestoreTimesheetAccess_RepositorySucceeds_ReturnsTrue)
{
    EXPECT_CALL(*mockRepo, restoreTimesheetAccess(1, "2024-01-01"))
        .WillOnce(Return(true));

    EXPECT_NO_THROW({
        service->restoreTimesheetAccess(1, "2024-01-01");
    });
}
