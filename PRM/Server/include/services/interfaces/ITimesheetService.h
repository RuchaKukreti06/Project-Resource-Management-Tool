#pragma once

#include <vector>
#include <string>
#include <optional>

#include "dto/request/SubmitTimesheetRequest.h"
#include "dto/response/TimesheetResponse.h"
#include "dto/response/TeamTimesheetResponse.h"
#include "dto/response/TimesheetDetailResponse.h"
#include "repositories/ITimesheetRepository.h"

class ITimesheetService {
public:
    virtual ~ITimesheetService() = default;
    virtual void submitTimesheet(const SubmitTimesheetRequest& req) = 0;
    virtual std::vector<TimesheetResponse> getEmployeeTimesheets(int employeeId) = 0;
    virtual std::optional<Timesheet> getTimesheetById(int timesheetId) = 0;
    virtual std::vector<TeamTimesheetResponse> getTeamTimesheets(int managerUserId, const std::string& weekStartDate) = 0;
    virtual std::vector<TimesheetDetailResponse> getTimesheetDetails(int timesheetId) = 0;
    virtual std::vector<int> getMissedTimesheetEmployeeIds(const std::string& weekStartDate) = 0;
};
