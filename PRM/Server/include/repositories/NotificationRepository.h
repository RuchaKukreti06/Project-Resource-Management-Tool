#ifndef SERVER_REPOSITORIES_NOTIFICATION_REPOSITORY_H
#define SERVER_REPOSITORIES_NOTIFICATION_REPOSITORY_H

#include "database/Database.h"
#include "repositories/INotificationRepository.h"

class NotificationRepository : public INotificationRepository
{
   public:
    explicit NotificationRepository(database::Database& database);

    int getNotificationStage(int userId, const std::string& weekStartDate) override;
    bool setNotificationStage(int userId, const std::string& weekStartDate, int stage) override;
    bool lockTimesheetAccess(int userId, const std::string& weekStartDate) override;
    bool restoreTimesheetAccess(int userId, const std::string& weekStartDate) override;
    bool isTimesheetAccessLocked(int userId) override;

   private:
    void ensureSchema();

    database::Database& database_;
};

#endif