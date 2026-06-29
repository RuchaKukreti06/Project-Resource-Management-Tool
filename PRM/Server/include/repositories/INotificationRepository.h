#ifndef SERVER_REPOSITORIES_INOTIFICATION_REPOSITORY_H
#define SERVER_REPOSITORIES_INOTIFICATION_REPOSITORY_H

#include <string>

class INotificationRepository
{
   public:
    virtual ~INotificationRepository() = default;

    virtual int getNotificationStage(int userId, const std::string& weekStartDate) = 0;
    virtual bool setNotificationStage(int userId, const std::string& weekStartDate, int stage) = 0;
    virtual bool lockTimesheetAccess(int userId, const std::string& weekStartDate) = 0;
    virtual bool restoreTimesheetAccess(int userId, const std::string& weekStartDate) = 0;
    virtual bool isTimesheetAccessLocked(int userId) = 0;
};

#endif