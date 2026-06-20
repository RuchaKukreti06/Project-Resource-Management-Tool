#pragma once

#include <gmock/gmock.h>
#include "services/interfaces/INotificationService.h"
#include <string>
#include <vector>

class MockNotificationService : public INotificationService {
public:
    MOCK_METHOD(void, processMissedTimesheetNotifications, (const std::string& weekStartDate, const std::vector<int>& missedUserIds), (override));
    MOCK_METHOD(void, restoreTimesheetAccess, (int userId, const std::string& weekStartDate), (override));
    MOCK_METHOD(bool, isTimesheetAccessLocked, (int userId), (const, override));
};
