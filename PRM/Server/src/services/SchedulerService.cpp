#include "services/SchedulerService.h"

#include <ctime>
#include <spdlog/spdlog.h>
#include <atomic>

namespace
{

static std::atomic<bool> g_isSchedulerRunning{false};

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

SchedulerService::SchedulerService(std::shared_ptr<IEmployeeService> employeeService,
                                   std::shared_ptr<IProjectService> projectService,
                                   std::shared_ptr<IAllocationService> allocationService,
                         std::shared_ptr<ITimesheetService> timesheetService,
                         std::shared_ptr<INotificationService> notificationService)
    : employeeService_(std::move(employeeService)),
      projectService_(std::move(projectService)),
      allocationService_(std::move(allocationService)),
    timesheetService_(std::move(timesheetService)),
    notificationService_(std::move(notificationService))
{
}

void SchedulerService::runRecomputationJob(const std::string& todayDate)
{
    bool expected = false;
    if (!g_isSchedulerRunning.compare_exchange_strong(expected, true))
    {
        spdlog::warn("Scheduler job is already running. Skipping execution.");
        return;
    }

    try
    {
        spdlog::info("Scheduler: running utilisation recomputation for {}", todayDate);

        // 1. Recompute employee bench/allocated status
        const auto employees = employeeService_->getAllEmployees();
        for (const auto& employee : employees)
        {
            allocationService_->recomputeEmployeeStatus(employee.id, todayDate);
        }

        // 2. Recompute project health
        projectService_->recomputeProjectHealth(todayDate);

        // 3. Flag missed timesheets for previous week
        flagMissedTimesheets(previousMondayIso(todayDate));

        spdlog::info("Scheduler: job complete.");
        g_isSchedulerRunning = false;
    }
    catch (...)
    {
        g_isSchedulerRunning = false;
        throw;
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
