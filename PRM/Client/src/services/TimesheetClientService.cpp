#include "services/TimesheetClientService.h"

TimesheetClientService::TimesheetClientService(IApiClient& apiClient)
    : apiClient_(apiClient)
{
}

ApiEmptyResponse TimesheetClientService::submitTimesheet(const SubmitTimesheetRequest& request)
{
    return ApiEmptyResponse::fromJson(apiClient_.post("/timesheets", request.toJson()));
}

ApiListResponse<TimesheetDTO> TimesheetClientService::getEmployeeTimesheets(int employeeId)
{
    return ApiListResponse<TimesheetDTO>::fromJson(apiClient_.get("/employees/" + std::to_string(employeeId) + "/timesheets"));
}

ApiListResponse<ManagerTimesheetDTO> TimesheetClientService::getManagerTimesheets(int managerId, const std::string& weekStartDate)
{
    std::string endpoint = "/managers/" + std::to_string(managerId) + "/timesheets";
    if (!weekStartDate.empty())
    {
        endpoint += "?week_start_date=" + weekStartDate;
    }
    return ApiListResponse<ManagerTimesheetDTO>::fromJson(apiClient_.get(endpoint));
}

ApiEmptyResponse TimesheetClientService::restoreTimesheetAccess(int employeeId, const std::string& weekStartDate)
{
    return ApiEmptyResponse::fromJson(apiClient_.put("/timesheets/access/restore", {
        {"employee_id", employeeId},
        {"week_start_date", weekStartDate}
    }));
}

ApiListResponse<TimesheetEntryDTO> TimesheetClientService::getTimesheetEntries(int timesheetId)
{
    return ApiListResponse<TimesheetEntryDTO>::fromJson(apiClient_.get("/timesheets/" + std::to_string(timesheetId)));
}
