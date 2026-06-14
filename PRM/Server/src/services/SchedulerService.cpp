#include "services/SchedulerService.h"

#include <ctime>
#include <spdlog/spdlog.h>

namespace
{

// Returns the Monday of the previous week as YYYY-MM-DD
std::string previousMondayIso(const std::string& todayDate)
{
    int year = 0, month = 0, day = 0;
    if (std::sscanf(todayDate.c_str(), "%d-%d-%d", &year, &month, &day) != 3)
        return todayDate;

    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon  = month - 1;
    tm.tm_mday = day;
    std::mktime(&tm);

    // tm_wday: 0=Sun,1=Mon,...,6=Sat  → days since last Monday
    int daysSinceMon = (tm.tm_wday == 0) ? 6 : tm.tm_wday - 1;
    tm.tm_mday -= (daysSinceMon + 7);  // go back to previous week's Monday
    std::mktime(&tm);

    char buf[11] = {};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return buf;
}

}  // namespace

SchedulerService::SchedulerService(std::shared_ptr<EmployeeService> employeeService,
                                   std::shared_ptr<ProjectService> projectService,
                                   std::shared_ptr<AllocationService> allocationService,
                         std::shared_ptr<TimesheetService> timesheetService,
                         std::shared_ptr<NotificationService> notificationService)
    : employeeService_(std::move(employeeService)),
      projectService_(std::move(projectService)),
      allocationService_(std::move(allocationService)),
    timesheetService_(std::move(timesheetService)),
    notificationService_(std::move(notificationService))
{
}

void SchedulerService::runRecomputationJob(const std::string& todayDate)
{
    spdlog::info("Scheduler: running utilisation recomputation for {}", todayDate);

    // 1. Recompute employee bench/allocated status
    const auto employees = employeeService_->getAllEmployees();
    for (const auto& employee : employees)
    {
        allocationService_->recomputeEmployeeStatus(employee.id, todayDate);
    }

    // 2. Recompute project health
    recomputeProjectHealth(todayDate);

    // 3. Flag missed timesheets for previous week
    flagMissedTimesheets(previousMondayIso(todayDate));

    spdlog::info("Scheduler: job complete.");
}

std::string SchedulerService::computeProjectHealth(int projectId, const std::string& todayDate)
{
    const auto milestones = projectService_->getProjectMilestones(projectId);

    bool hasOverdue     = false;
    bool hasInProgress  = false;

    for (const auto& m : milestones)
    {
        if (m.status == "DONE")
            continue;

        if (m.dueDate < todayDate)
        {
            hasOverdue = true;
        }
        if (m.status == "IN_PROGRESS")
        {
            hasInProgress = true;
        }
    }

    if (hasOverdue)
        return "AT_RISK";
    if (hasInProgress)
        return "ATTENTION";
    return "ON_TRACK";
}

void SchedulerService::recomputeProjectHealth(const std::string& todayDate)
{
    spdlog::info("Scheduler: recomputing project health for {}", todayDate);
    const auto projects = projectService_->getAllProjects();

    for (const auto& project : projects)
    {
        if (project.status == "COMPLETED" || project.status == "ON_HOLD")
            continue;

        const std::string health = computeProjectHealth(project.id, todayDate);
        if (health != project.healthStatus)
        {
            projectService_->updateProjectHealth(project.id, health);
            spdlog::info("Scheduler: project {} health updated to {}", project.id, health);
        }
    }
}

void SchedulerService::flagMissedTimesheets(const std::string& weekStartDate)
{
    spdlog::info("Scheduler: flagging missed timesheets for week {}", weekStartDate);
    const auto missedUserIds = timesheetService_->getMissedTimesheetEmployeeIds(weekStartDate);
    spdlog::info("Scheduler: {} employees missed timesheets for week {}", missedUserIds.size(),
                 weekStartDate);

    if (notificationService_ && !missedUserIds.empty())
    {
        notificationService_->processMissedTimesheetNotifications(weekStartDate, missedUserIds);
    }
}
