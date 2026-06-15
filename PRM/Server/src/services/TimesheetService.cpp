#include "services/TimesheetService.h"

#include <cstdio>
#include <ctime>
#include <unordered_map>

TimesheetService::TimesheetService(std::shared_ptr<ITimesheetRepository> timesheetRepository,
                                   std::shared_ptr<IEmployeeRepository> employeeRepository,
                                   std::shared_ptr<IAllocationRepository> allocationRepository)
    : timesheetRepository_(std::move(timesheetRepository)),
      employeeRepository_(std::move(employeeRepository)),
      allocationRepository_(std::move(allocationRepository))
{
}

std::string TimesheetService::computeWeekEndDate(const std::string& weekStartDate) const
{
    int year = 0;
    int month = 0;
    int day = 0;
    if (std::sscanf(weekStartDate.c_str(), "%d-%d-%d", &year, &month, &day) != 3)
    {
        return weekStartDate;
    }

    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_mday += 6;
    std::mktime(&tm);

    char buffer[11] = {0};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm);
    return buffer;
}

bool TimesheetService::submitTimesheet(int employeeId, const std::string& weekStartDate,
                                       const std::vector<TimesheetLineInput>& lines,
                                       int maxWeeklyHours, std::string& message)
{
    const auto employee = employeeRepository_->getEmployeeById(employeeId);
    if (employee.id == 0 || !employee.isActive)
    {
        message = "Employee not found or inactive.";
        return false;
    }

    if (timesheetRepository_->existsTimesheetForWeek(employeeId, weekStartDate))
    {
        message = "Timesheet already exists for this week.";
        return false;
    }

    const std::string weekEndDate = computeWeekEndDate(weekStartDate);
    const auto allocations =
        timesheetRepository_->getActiveAllocationsForWeek(employeeId, weekStartDate, weekEndDate);

    if (allocations.empty())
    {
        message = "No active allocations for this week.";
        return false;
    }

    std::unordered_map<int, int> maxProjectHours;
    for (const auto& allocation : allocations)
    {
        const int allowed = (allocation.utilizationPercentage * maxWeeklyHours) / 100;
        const auto found = maxProjectHours.find(allocation.projectId);
        if (found == maxProjectHours.end() || found->second < allowed)
        {
            maxProjectHours[allocation.projectId] = allowed;
        }
    }

    int total = 0;
    for (const auto& line : lines)
    {
        const auto limitIt = maxProjectHours.find(line.projectId);
        if (limitIt == maxProjectHours.end())
        {
            message = "Hours cannot be logged for unallocated project.";
            return false;
        }

        if (line.hoursWorked > limitIt->second)
        {
            message = "Project hours exceed allowed allocation limit.";
            return false;
        }

        total += line.hoursWorked;
    }

    if (total > maxWeeklyHours)
    {
        message = "Total hours exceed weekly maximum.";
        return false;
    }

    const bool ok = timesheetRepository_->createTimesheetWithLines(employeeId, weekStartDate, lines);
    message = ok ? "Timesheet submitted." : "Failed to submit timesheet.";
    return ok;
}

std::vector<Timesheet> TimesheetService::getEmployeeTimesheets(int employeeId)
{
    return timesheetRepository_->getTimesheetsByEmployee(employeeId);
}

std::vector<TeamTimesheetRow> TimesheetService::getTeamTimesheets(int managerUserId,
                                                                   const std::string& weekStartDate)
{
    return timesheetRepository_->getTeamTimesheets(managerUserId, weekStartDate);
}

std::vector<TimesheetDetailRow> TimesheetService::getTimesheetDetails(int timesheetId)
{
    return timesheetRepository_->getTimesheetDetails(timesheetId);
}

std::vector<int> TimesheetService::getMissedTimesheetEmployeeIds(const std::string& weekStartDate)
{
    return timesheetRepository_->getEmployeesWithMissedTimesheets(weekStartDate);
}
