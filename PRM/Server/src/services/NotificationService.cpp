#include "services/NotificationService.h"

#include <spdlog/spdlog.h>

#include <sstream>
#include "exceptions/Exceptions.h"
#include <nlohmann/json.hpp>

NotificationService::NotificationService(
    std::shared_ptr<INotificationRepository> notificationRepository,
    std::shared_ptr<IUserRepository> userRepository, std::shared_ptr<EmailService> emailService,
    std::shared_ptr<IProjectService> projectService,
    std::shared_ptr<AIService> aiService,
    std::shared_ptr<ISystemConfigRepository> systemConfigRepo)
    : notificationRepository_(std::move(notificationRepository)),
      userRepository_(std::move(userRepository)),
      emailService_(std::move(emailService)),
      projectService_(std::move(projectService)),
      aiService_(std::move(aiService)),
      systemConfigRepo_(std::move(systemConfigRepo))
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

void NotificationService::processProjectAtRisk(int projectId, const std::string& todayDate)
{
    spdlog::info("NotificationService: processing AT_RISK project {}", projectId);
    try
    {
        if (!projectService_)
        {
            spdlog::error("Project service dependency is missing in NotificationService.");
            return;
        }

        auto projectOpt = projectService_->getProjectById(projectId);
        if (!projectOpt)
        {
            spdlog::error("Project not found: {}", projectId);
            return;
        }
        const auto& project = *projectOpt;

        if (project.managerId <= 0)
        {
            spdlog::warn("Project {} has no manager assigned. Skipping AT_RISK email.", projectId);
            return;
        }

        User manager = userRepository_->getUserById(project.managerId);
        if (manager.id <= 0)
        {
            spdlog::error("Manager {} not found for project {}", project.managerId, projectId);
            return;
        }
        if (manager.email.empty())
        {
            spdlog::warn("Manager {} for project {} has no email. Skipping.", project.managerId, projectId);
            return;
        }

        std::string aiSummary = "No AI summary available.";
        std::string suggestedHelp = "No automatic suggestions available.";

        if (aiService_ && systemConfigRepo_)
        {
            int maxWeeklyHours = 40;
            try
            {
                auto config = systemConfigRepo_->getConfig();
                maxWeeklyHours = config.maxWeeklyHours;
                if (!config.llmApiKey.empty())
                {
                    aiSummary = aiService_->riskSummary(projectId, todayDate, config.llmApiKey, config.llmProvider);
                    suggestedHelp = aiService_->skillMatch("Available employees who can help with project delays", maxWeeklyHours, config.llmApiKey, config.llmProvider);
                }
                
                // If API call failed or keys were empty, fetch the deterministic fallback
                if (aiSummary.empty() || config.llmApiKey.empty())
                {
                    aiSummary = aiService_->riskSummary(projectId, todayDate, "", "");
                }
                
                if (suggestedHelp.empty() || config.llmApiKey.empty())
                {
                    suggestedHelp = aiService_->skillMatch("Available employees who can help with project delays", maxWeeklyHours, "", "");
                }
            }
            catch (const std::exception& e)
            {
                spdlog::warn("AI fallback triggered for project {} due to error: {}", projectId, e.what());
                // Ensure we still have a fallback if exception thrown
                if (aiSummary.empty() || aiSummary == "No AI summary available.") 
                    aiSummary = aiService_->riskSummary(projectId, todayDate, "", "");
                if (suggestedHelp.empty() || suggestedHelp == "No automatic suggestions available.") 
                    suggestedHelp = aiService_->skillMatch("Available employees who can help with project delays", maxWeeklyHours, "", "");
            }
        }

        const auto milestones = projectService_->getProjectMilestones(projectId);
        std::stringstream body;
        body << "Hello " << manager.fullName << ",\n\n"
             << "This is an automated alert regarding your project: " << project.name << ".\n\n"
             << "CURRENT STATUS: AT_RISK\n\n";

        bool hasOverdue = false;
        body << "Overdue or Risky Milestones:\n";
        for (const auto& ms : milestones)
        {
            if (ms.status != "DONE" && ms.dueDate < todayDate)
            {
                hasOverdue = true;
                body << "- " << ms.title << " (Due: " << ms.dueDate << ")\n";
            }
        }
        if (!hasOverdue)
        {
            body << "- (No specific overdue milestones found)\n";
        }

        std::string formattedHelp = suggestedHelp;
        try {
            auto jsonHelp = nlohmann::json::parse(suggestedHelp);
            if (jsonHelp.is_array() && !jsonHelp.empty()) {
                std::stringstream sb;
                for (const auto& item : jsonHelp) {
                    sb << "- " << item.value("name", "Unknown") 
                       << " (ID: " << item.value("employee_id", 0) << ")\n"
                       << "  Reason: " << item.value("reason", "") << "\n";
                }
                formattedHelp = sb.str();
            } else if (jsonHelp.is_array() && jsonHelp.empty()) {
                formattedHelp = "No available employees found matching the criteria.";
            }
        } catch (...) {
            // Not a valid JSON array, keep the raw string
        }

        body << "\nAI Risk Summary:\n" << aiSummary << "\n\n";
        body << "Suggested Help:\n" << formattedHelp << "\n\n";
        body << "Please take necessary actions to bring this project back on track.\n\n"
             << "Regards,\nPRM System Scheduler\n";

        std::string subject = "Project At-Risk Alert: " + project.name;
        
        bool sent = sendEmail(manager.email, subject, body.str());
        if (!sent)
        {
            spdlog::error("Failed to send AT_RISK email to {} for project {}", manager.email, projectId);
        }
        else
        {
            spdlog::info("Sent AT_RISK email to {} for project {}", manager.email, projectId);
        }
    }
    catch (const std::exception& e)
    {
        spdlog::error("Unhandled exception in processProjectAtRisk for project {}: {}", projectId, e.what());
    }
}
