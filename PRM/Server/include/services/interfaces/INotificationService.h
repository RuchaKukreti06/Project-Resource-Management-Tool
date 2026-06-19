#pragma once

#include <vector>
#include <string>

class INotificationService {
public:
    virtual ~INotificationService() = default;
    virtual void processMissedTimesheetNotifications(const std::string& weekStartDate, const std::vector<int>& missedUserIds) = 0;
    virtual bool restoreTimesheetAccess(int userId, const std::string& weekStartDate, std::string& message) = 0;
    virtual bool isTimesheetAccessLocked(int userId) const = 0;
};
