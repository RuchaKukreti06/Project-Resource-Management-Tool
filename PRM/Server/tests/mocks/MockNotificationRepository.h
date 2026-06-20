#pragma once

#include <gmock/gmock.h>
#include "repositories/INotificationRepository.h"
#include <string>

class MockNotificationRepository : public INotificationRepository {
public:
    MOCK_METHOD(int, getNotificationStage, (int userId, const std::string& weekStartDate), (override));
    MOCK_METHOD(bool, setNotificationStage, (int userId, const std::string& weekStartDate, int stage), (override));
    MOCK_METHOD(bool, lockTimesheetAccess, (int userId, const std::string& weekStartDate), (override));
    MOCK_METHOD(bool, restoreTimesheetAccess, (int userId, const std::string& weekStartDate), (override));
    MOCK_METHOD(bool, isTimesheetAccessLocked, (int userId), (override));
};
