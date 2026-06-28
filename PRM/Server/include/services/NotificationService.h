#ifndef SERVER_SERVICES_NOTIFICATION_SERVICE_H
#define SERVER_SERVICES_NOTIFICATION_SERVICE_H

#include <memory>
#include <string>
#include <vector>

#include "repositories/INotificationRepository.h"
#include "repositories/IUserRepository.h"
#include "repositories/ISystemConfigRepository.h"
#include "services/EmailService.h"
#include "services/AIService.h"
#include "services/interfaces/INotificationService.h"
#include "services/interfaces/IProjectService.h"

class NotificationService : public INotificationService
{
   public:
    NotificationService(std::shared_ptr<INotificationRepository> notificationRepository,
                        std::shared_ptr<IUserRepository> userRepository,
                        std::shared_ptr<EmailService> emailService,
                        std::shared_ptr<IProjectService> projectService,
                        std::shared_ptr<AIService> aiService,
                        std::shared_ptr<ISystemConfigRepository> systemConfigRepo);

    void processMissedTimesheetNotifications(const std::string& weekStartDate,
                                             const std::vector<int>& missedUserIds) override;
    void processProjectAtRisk(int projectId, const std::string& todayDate) override;
    void restoreTimesheetAccess(int userId, const std::string& weekStartDate) override;
    bool isTimesheetAccessLocked(int userId) const override;

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
    std::shared_ptr<IProjectService> projectService_;
    std::shared_ptr<AIService> aiService_;
    std::shared_ptr<ISystemConfigRepository> systemConfigRepo_;
};

#endif