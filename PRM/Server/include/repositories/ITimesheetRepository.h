#pragma once

#include <string>
#include <vector>
#include <optional>

#include "models/Allocations.h"
#include "models/Timesheet.h"
#include "models/TimesheetLines.h"

struct TimesheetLineInput
{
    int projectId = 0;
    int hoursWorked = 0;
    std::vector<std::string> tags;
};

struct TeamTimesheetRow
{
    std::string employeeName;
    int employeeId = 0;
    int projectId = 0;
    std::string projectName;
    int hours = 0;
    std::string status;
};

struct TimesheetDetailRow
{
    int projectId = 0;
    std::string projectName;
    int hours = 0;
    std::string tags; // comma separated
};

class ITimesheetRepository
{
   public:
    virtual ~ITimesheetRepository() = default;

    virtual bool existsTimesheetForWeek(int employeeId, const std::string& weekStartDate) = 0;
    virtual bool createTimesheetWithLines(int employeeId, const std::string& weekStartDate,
                                          const std::vector<TimesheetLineInput>& lines) = 0;
    virtual std::vector<Allocation> getActiveAllocationsForWeek(int employeeId,
                                                                 const std::string& weekStartDate,
                                                                 const std::string& weekEndDate) = 0;
    virtual std::optional<Timesheet> getTimesheetById(int timesheetId) = 0;
    virtual std::vector<Timesheet> getTimesheetsByEmployee(int employeeId) = 0;
    virtual std::vector<TeamTimesheetRow> getTeamTimesheets(int managerUserId,
                                                             const std::string& weekStartDate) = 0;
    virtual int getProjectHoursForWeek(int projectId, const std::string& weekStartDate) = 0;
    virtual std::vector<TimesheetDetailRow> getTimesheetDetails(int timesheetId) = 0;
    // Returns employee IDs of all allocated employees who have NOT submitted a timesheet for weekStart
    virtual std::vector<int> getEmployeesWithMissedTimesheets(const std::string& weekStartDate) = 0;
};
