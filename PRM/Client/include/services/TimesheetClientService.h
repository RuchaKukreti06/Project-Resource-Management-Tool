#ifndef TIMESHEET_CLIENT_SERVICE_H
#define TIMESHEET_CLIENT_SERVICE_H

#include "api/IApiClient.h"
#include "dto/TimesheetDTO.h"
#include "dto/ApiResponse.h"
#include <string>

class TimesheetClientService
{
private:
    IApiClient& apiClient_;

public:
    explicit TimesheetClientService(IApiClient& apiClient);

    ApiEmptyResponse submitTimesheet(const SubmitTimesheetRequest& request);
    ApiListResponse<TimesheetDTO> getEmployeeTimesheets(int employeeId);
    ApiListResponse<ManagerTimesheetDTO> getManagerTimesheets(int managerId, const std::string& weekStartDate = "");
    ApiEmptyResponse restoreTimesheetAccess(int employeeId, const std::string& weekStartDate);
    ApiListResponse<TimesheetEntryDTO> getTimesheetEntries(int timesheetId);
};

#endif // TIMESHEET_CLIENT_SERVICE_H
