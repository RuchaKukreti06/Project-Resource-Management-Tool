#pragma once

#include <gmock/gmock.h>
#include "repositories/ITimesheetRepository.h"

class MockTimesheetRepository : public ITimesheetRepository
{
public:
    MOCK_METHOD(bool, existsTimesheetForWeek, (int employeeId, const std::string& weekStartDate), (override));
    MOCK_METHOD(bool, createTimesheetWithLines, (int employeeId, const std::string& weekStartDate, const std::vector<TimesheetLineInput>& lines), (override));
    MOCK_METHOD(std::vector<Allocation>, getActiveAllocationsForWeek, (int employeeId, const std::string& weekStartDate, const std::string& weekEndDate), (override));
    MOCK_METHOD(std::vector<Timesheet>, getTimesheetsByEmployee, (int employeeId), (override));
    MOCK_METHOD(std::vector<TeamTimesheetRow>, getTeamTimesheets, (int managerUserId, const std::string& weekStartDate), (override));
    MOCK_METHOD(int, getProjectHoursForWeek, (int projectId, const std::string& weekStartDate), (override));
    MOCK_METHOD(std::vector<TimesheetDetailRow>, getTimesheetDetails, (int timesheetId), (override));
    MOCK_METHOD(std::vector<int>, getEmployeesWithMissedTimesheets, (const std::string& weekStartDate), (override));
};
