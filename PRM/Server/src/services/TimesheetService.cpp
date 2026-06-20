#include "services/TimesheetService.h"
#include "dto/DTOMapper.h"

#include <cstdio>
#include <ctime>
#include <unordered_map>

TimesheetService::TimesheetService(std::shared_ptr<ITimesheetRepository> timesheetRepository,
                                   std::shared_ptr<IEmployeeRepository> employeeRepository,
                                   std::shared_ptr<IAllocationRepository> allocationRepository,
                                   std::shared_ptr<INotificationService> notificationService)
    : timesheetRepository_(std::move(timesheetRepository)),
      employeeRepository_(std::move(employeeRepository)),
      allocationRepository_(std::move(allocationRepository)),
      notificationService_(std::move(notificationService))
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
    tm.tm_mon  = month - 1;
    tm.tm_mday = day;
    tm.tm_mday += 6;
    std::mktime(&tm);

    char buffer[11] = {0};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm);
    return buffer;
}

bool TimesheetService::submitTimesheet(const SubmitTimesheetRequest& req, std::string& message)
{
    std::vector<TimesheetLineInput> lines;
    for (const auto& dtoLine : req.lines)
    {
        TimesheetLineInput input;
        input.projectId = dtoLine.projectId;
        input.hoursWorked = dtoLine.hoursWorked;
        input.tags = dtoLine.tags;
        lines.push_back(input);
    }

    if (!timesheetValidator_.validateSubmit(req.employeeId, req.weekStartDate, lines, req.maxWeeklyHours, message))
    {
        return false;
    }

    const auto employee = employeeRepository_->getEmployeeById(req.employeeId);
    if (employee.id == 0 || !employee.isActive)
    {
        message = "Employee not found or inactive.";
        return false;
    }

    if (notificationService_ && employee.user_id > 0 &&
        notificationService_->isTimesheetAccessLocked(employee.user_id))
    {
        message = "Timesheet submission access is temporarily restricted.";
        return false;
    }

    if (timesheetRepository_->existsTimesheetForWeek(req.employeeId, req.weekStartDate))
    {
        message = "Timesheet already exists for this week.";
        return false;
    }

    const std::string weekEndDate = computeWeekEndDate(req.weekStartDate);
    const auto allocations =
        timesheetRepository_->getActiveAllocationsForWeek(req.employeeId, req.weekStartDate, weekEndDate);

    if (allocations.empty())
    {
        message = "No active allocations for this week.";
        return false;
    }

    std::unordered_map<int, int> maxProjectHours;
    for (const auto& allocation : allocations)
    {
        const int allowed = (allocation.utilizationPercentage * req.maxWeeklyHours) / 100;
        const auto found  = maxProjectHours.find(allocation.projectId);
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

    if (total > req.maxWeeklyHours)
    {
        message = "Total hours exceed weekly maximum.";
        return false;
    }

    const bool ok = timesheetRepository_->createTimesheetWithLines(req.employeeId, req.weekStartDate, lines);
    message = ok ? "Timesheet submitted." : "Failed to submit timesheet.";
    return ok;
}

std::vector<TimesheetResponse> TimesheetService::getEmployeeTimesheets(int employeeId)
{
    return DTOMapper::mapToTimesheetResponse(timesheetRepository_->getTimesheetsByEmployee(employeeId));
}

std::vector<TeamTimesheetResponse> TimesheetService::getTeamTimesheets(int managerUserId,
                                                                 const std::string& weekStartDate)
{
    auto rows = timesheetRepository_->getTeamTimesheets(managerUserId, weekStartDate);
    return DTOMapper::mapToTeamTimesheetResponse(rows);
}

std::vector<TimesheetDetailResponse> TimesheetService::getTimesheetDetails(int timesheetId)
{
    auto details = timesheetRepository_->getTimesheetDetails(timesheetId);
    return DTOMapper::mapToTimesheetDetailResponse(details);
}

std::vector<int> TimesheetService::getMissedTimesheetEmployeeIds(const std::string& weekStartDate)
{
    return timesheetRepository_->getEmployeesWithMissedTimesheets(weekStartDate);
}
