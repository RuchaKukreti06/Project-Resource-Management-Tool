#ifndef SERVER_SERVICES_NOTIFICATION_SERVICE_H
#define SERVER_SERVICES_NOTIFICATION_SERVICE_H

#include <memory>
#include <string>
#include <vector>

#include "repositories/INotificationRepository.h"
#include "repositories/IUserRepository.h"
#include "services/EmailService.h"

class NotificationService
{
   public:
    NotificationService(std::shared_ptr<INotificationRepository> notificationRepository,
                        std::shared_ptr<IUserRepository> userRepository,
                        std::shared_ptr<EmailService> emailService);

    void processMissedTimesheetNotifications(const std::string& weekStartDate,
                                             const std::vector<int>& missedUserIds);
    bool restoreTimesheetAccess(int userId, const std::string& weekStartDate,
                                std::string& message);

   private:
    bool sendEmail(const std::string& to, const std::string& subject, const std::string& body);
    bool sendReminderEmail(const User& employee, int reminderNumber,
                           const std::string& weekStartDate);
    void sendFreezeNotification(const User& employee, const User& manager,
                                const std::string& weekStartDate);
    void processUser(const User& employee, const std::string& weekStartDate);

    std::shared_ptr<INotificationRepository> notificationRepository_;
    std::shared_ptr<IUserRepository> userRepository_;
    std::shared_ptr<EmailService> emailService_;
};

#endif