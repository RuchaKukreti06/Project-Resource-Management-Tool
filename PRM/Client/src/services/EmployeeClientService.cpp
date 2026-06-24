#include "services/EmployeeClientService.h"

EmployeeClientService::EmployeeClientService(IApiClient& apiClient)
    : apiClient_(apiClient)
{
}

ApiListResponse<EmployeeDTO> EmployeeClientService::viewAllEmployees()
{
    return ApiListResponse<EmployeeDTO>::fromJson(apiClient_.get("/employees"));
}

ApiEmptyResponse EmployeeClientService::updateEmployee(int employeeId, const UpdateEmployeeRequest& request)
{
    return ApiEmptyResponse::fromJson(apiClient_.put("/employees/" + std::to_string(employeeId), request.toJson()));
}

ApiEmptyResponse EmployeeClientService::deactivateEmployee(int employeeId)
{
    return ApiEmptyResponse::fromJson(apiClient_.patch("/employees/" + std::to_string(employeeId) + "/deactivate", nlohmann::json::object()));
}

ApiListResponse<SkillDTO> EmployeeClientService::getEmployeeSkills(int employeeId)
{
    return ApiListResponse<SkillDTO>::fromJson(apiClient_.get("/employees/" + std::to_string(employeeId) + "/skills"));
}

ApiEmptyResponse EmployeeClientService::addSkill(int employeeId, const AddSkillRequest& request)
{
    return ApiEmptyResponse::fromJson(apiClient_.post("/employees/" + std::to_string(employeeId) + "/skills", request.toJson()));
}

ApiEmptyResponse EmployeeClientService::updateSkill(int employeeId, int skillId, const UpdateSkillRequest& request)
{
    return ApiEmptyResponse::fromJson(apiClient_.put("/employees/" + std::to_string(employeeId) + "/skills/" + std::to_string(skillId), request.toJson()));
}

ApiEmptyResponse EmployeeClientService::removeSkill(int employeeId, int skillId)
{
    return ApiEmptyResponse::fromJson(apiClient_.del("/employees/" + std::to_string(employeeId) + "/skills/" + std::to_string(skillId)));
}

ApiListResponse<EmployeeDTO> EmployeeClientService::getTeamEmployees(int managerId)
{
    return ApiListResponse<EmployeeDTO>::fromJson(apiClient_.get("/managers/" + std::to_string(managerId) + "/team"));
}

ApiEmptyResponse EmployeeClientService::assignManager(int employeeUserId, int managerUserId)
{
    return ApiEmptyResponse::fromJson(apiClient_.put("/users/" + std::to_string(employeeUserId) + "/assign-manager", {{"manager_id", managerUserId}}));
}
