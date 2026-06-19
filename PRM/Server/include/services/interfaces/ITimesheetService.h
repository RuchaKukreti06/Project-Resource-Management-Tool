#pragma once

#include <vector>
#include <string>
#include "models/Timesheet.h"
#include "repositories/ITimesheetRepository.h"

class ITimesheetService {
public:
    virtual ~ITimesheetService() = default;
    virtual bool submitTimesheet(int employeeId, const std::string& weekStartDate,
                                 const std::vector<TimesheetLineInput>& lines, int maxWeeklyHours, std::string& message) = 0;
    virtual std::vector<Timesheet> getEmployeeTimesheets(int employeeId) = 0;
    virtual std::vector<TeamTimesheetRow> getTeamTimesheets(int managerUserId, const std::string& weekStartDate) = 0;
    virtual std::vector<TimesheetDetailRow> getTimesheetDetails(int timesheetId) = 0;
    virtual std::vector<int> getMissedTimesheetEmployeeIds(const std::string& weekStartDate) = 0;
    virtual bool restoreTimesheetAccess(int userId, const std::string& weekStartDate, std::string& message) = 0;
};
