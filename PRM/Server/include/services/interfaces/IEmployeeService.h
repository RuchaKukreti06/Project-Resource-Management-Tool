#pragma once

#include <vector>
#include <optional>
#include <string>
#include "models/Employee.h"
#include "repositories/IEmployeeRepository.h"

#include "dto/request/EmployeeCreateRequest.h"
#include "dto/request/AddSkillRequest.h"
#include "dto/request/UpdateSkillRequest.h"
#include "dto/response/EmployeeResponse.h"
#include "dto/response/EmployeeSkillResponse.h"

class IEmployeeService {
public:
    virtual ~IEmployeeService() = default;
    virtual void createEmployeeProfile(const EmployeeCreateRequest& req) = 0;
    virtual void updateEmployeeProfile(const Employee& employee) = 0;
    virtual void deactivateEmployee(int employeeId, const std::string& todayDate) = 0;
    virtual std::vector<EmployeeResponse> getAllEmployees() = 0;
    virtual std::vector<EmployeeResponse> getTeamEmployees(int managerUserId) = 0;
    virtual std::optional<Employee> getEmployeeById(int employeeId) = 0;
    virtual void addSkill(const AddSkillRequest& req) = 0;
    virtual void updateSkill(const UpdateSkillRequest& req) = 0;
    virtual void removeSkill(int employeeId, int skillId) = 0;
    virtual std::vector<EmployeeSkillResponse> getSkills(int employeeId) = 0;
};
