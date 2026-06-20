#pragma once

#include <gmock/gmock.h>
#include "services/interfaces/ITimesheetService.h"
#include <string>
#include <vector>

class MockTimesheetService : public ITimesheetService {
public:
    MOCK_METHOD(void, submitTimesheet, (const SubmitTimesheetRequest& req), (override));
    MOCK_METHOD(std::vector<TimesheetResponse>, getEmployeeTimesheets, (int employeeId), (override));
    MOCK_METHOD(std::vector<TeamTimesheetResponse>, getTeamTimesheets, (int managerUserId, const std::string& weekStartDate), (override));
    MOCK_METHOD(std::vector<TimesheetDetailResponse>, getTimesheetDetails, (int timesheetId), (override));
    MOCK_METHOD(std::vector<int>, getMissedTimesheetEmployeeIds, (const std::string& weekStartDate), (override));
};
