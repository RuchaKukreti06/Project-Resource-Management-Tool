#pragma once

#include <gmock/gmock.h>
#include "services/interfaces/ITimesheetService.h"
#include <string>
#include <vector>

class MockTimesheetService : public ITimesheetService {
public:
    MOCK_METHOD(bool, submitTimesheet, (int employeeId, const std::string& weekStartDate, const std::vector<TimesheetLineInput>& lines, int maxWeeklyHours, std::string& message), (override));
    MOCK_METHOD(std::vector<Timesheet>, getEmployeeTimesheets, (int employeeId), (override));
    MOCK_METHOD(std::vector<TeamTimesheetRow>, getTeamTimesheets, (int managerUserId, const std::string& weekStartDate), (override));
    MOCK_METHOD(std::vector<TimesheetDetailRow>, getTimesheetDetails, (int timesheetId), (override));
    MOCK_METHOD(std::vector<int>, getMissedTimesheetEmployeeIds, (const std::string& weekStartDate), (override));
};
