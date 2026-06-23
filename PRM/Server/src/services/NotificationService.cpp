#include "services/NotificationService.h"

#include <spdlog/spdlog.h>

#include <sstream>
#include "exceptions/Exceptions.h"

NotificationService::NotificationService(
    std::shared_ptr<INotificationRepository> notificationRepository,
    std::shared_ptr<IUserRepository> userRepository, std::shared_ptr<EmailService> emailService)
    : notificationRepository_(std::move(notificationRepository)),
      userRepository_(std::move(userRepository)),
      emailService_(std::move(emailService))
{
}

bool NotificationService::sendEmail(const std::string& to, const std::string& subject,
                                    const std::string& body)
{
    if (!emailService_)
    {
        spdlog::error("Email service dependency is missing.");
        return false;
    }

    if (to.empty() || to.find('@') == std::string::npos || to.find('.') == std::string::npos)
    {
        spdlog::error("Invalid email format for recipient: {}", to);
        return false;
    }

    EmailMessage message;
    message.to = to;
    message.subject = subject;
    message.body = body;

    std::string errorMessage;
    const bool sent = emailService_->sendEmail(message, errorMessage);
    if (!sent)
    {
        spdlog::error("Failed to send email to {}: {}", to, errorMessage);
    }
    return sent;
}

bool NotificationService::sendReminderEmail(const User& employee, int reminderNumber,
                                            const std::string& weekStartDate)
{
    std::ostringstream subject;
    subject << "Timesheet Reminder " << reminderNumber << " - Week " << weekStartDate;

    std::ostringstream body;
    body << "Hello " << employee.fullName << ",\n\n";
    body << "Our records show that your timesheet for week starting " << weekStartDate
         << " has not been submitted.\n";
    body << "Please submit your timesheet at the earliest.\n\n";
    body << "Regards,\nPRM System";

    return sendEmail(employee.email, subject.str(), body.str());
}

void NotificationService::sendFreezeNotification(const User& employee, const User& manager,
                                                 const std::string& weekStartDate)
{
    std::ostringstream employeeBody;
    employeeBody << "Hello " << employee.fullName << ",\n\n";
    employeeBody << "Your timesheet submission access has been temporarily frozen because "
                 << "the timesheet for week starting " << weekStartDate
                 << " is still pending after reminders.\n";
    employeeBody << "Please contact your reporting manager to restore access after review.\n\n";
    employeeBody << "Regards,\nPRM System";

    sendEmail(employee.email,
              "Timesheet Access Frozen - Week " + weekStartDate,
              employeeBody.str());

    if (!manager.email.empty())
    {
        std::ostringstream managerBody;
        managerBody << "Hello " << manager.fullName << ",\n\n";
        managerBody << "Timesheet access for employee " << employee.fullName
                    << " has been frozen for week starting " << weekStartDate
                    << " due to non-submission after reminders.\n";
        managerBody << "Please review and restore access when appropriate.\n\n";
        managerBody << "Regards,\nPRM System";

        sendEmail(manager.email,
                  "Employee Timesheet Access Frozen - " + employee.fullName,
                  managerBody.str());
    }
}

void NotificationService::processUser(const User& employee, const std::string& weekStartDate)
{
    if (employee.id <= 0 || employee.email.empty())
    {
        return;
    }

    const int stage = notificationRepository_->getNotificationStage(employee.id, weekStartDate);

    if (stage <= 0)
    {
        if (sendReminderEmail(employee, 1, weekStartDate))
        {
            notificationRepository_->setNotificationStage(employee.id, weekStartDate, 1);
        }
        return;
    }

    if (stage == 1)
    {
        if (sendReminderEmail(employee, 2, weekStartDate))
        {
            notificationRepository_->setNotificationStage(employee.id, weekStartDate, 2);
        }
        return;
    }

    if (notificationRepository_->isTimesheetAccessLocked(employee.id))
    {
        return;
    }

    if (!notificationRepository_->lockTimesheetAccess(employee.id, weekStartDate))
    {
        return;
    }

    User manager;
    if (employee.managerId > 0)
    {
        manager = userRepository_->getUserById(employee.managerId);
    }

    sendFreezeNotification(employee, manager, weekStartDate);
}

void NotificationService::processMissedTimesheetNotifications(
    const std::string& weekStartDate, const std::vector<int>& missedUserIds)
{
    if (!notificationRepository_ || !userRepository_)
    {
        spdlog::error("Notification dependencies are not fully configured.");
        return;
    }

    for (const int userId : missedUserIds)
    {
        if (userId <= 0)
        {
            continue;
        }

        const User employee = userRepository_->getUserById(userId);
        processUser(employee, weekStartDate);
    }
}

void NotificationService::restoreTimesheetAccess(int userId, const std::string& weekStartDate)
{
    if (!notificationRepository_)
    {
        throw exceptions::DatabaseException("Notification repository is unavailable.");
    }
    if (userId <= 0 || weekStartDate.empty())
    {
        throw exceptions::ValidationException("user_id and week_start_date are required.");
    }

    const bool ok = notificationRepository_->restoreTimesheetAccess(userId, weekStartDate);
    if (!ok)
    {
        throw exceptions::DatabaseException("Failed to restore timesheet access.");
    }
}

bool NotificationService::isTimesheetAccessLocked(int userId) const
{
    if (!notificationRepository_)
    {
        return false;
    }
    return notificationRepository_->isTimesheetAccessLocked(userId);
}
