#ifndef EMPLOYEE_CLIENT_SERVICE_H
#define EMPLOYEE_CLIENT_SERVICE_H

#include "api/IApiClient.h"
#include "dto/EmployeeDTO.h"
#include "dto/ApiResponse.h"
#include <string>
#include <vector>

class EmployeeClientService
{
private:
    IApiClient& apiClient_;

public:
    explicit EmployeeClientService(IApiClient& apiClient);

    ApiListResponse<EmployeeDTO> viewAllEmployees();
    ApiEmptyResponse updateEmployee(int employeeId, const UpdateEmployeeRequest& request);
    ApiEmptyResponse deactivateEmployee(int employeeId);
    
    ApiListResponse<SkillDTO> getEmployeeSkills(int employeeId);
    ApiEmptyResponse addSkill(int employeeId, const AddSkillRequest& request);
    ApiEmptyResponse updateSkill(int employeeId, int skillId, const UpdateSkillRequest& request);
    ApiEmptyResponse removeSkill(int employeeId, int skillId);

    ApiListResponse<EmployeeDTO> getTeamEmployees(int managerId);
    ApiEmptyResponse assignManager(int employeeUserId, int managerUserId);
};

#endif // EMPLOYEE_CLIENT_SERVICE_H
