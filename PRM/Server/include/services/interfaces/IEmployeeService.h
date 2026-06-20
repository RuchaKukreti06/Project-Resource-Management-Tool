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
    virtual bool createEmployeeProfile(const EmployeeCreateRequest& req, std::string& message) = 0;
    virtual bool updateEmployeeProfile(const Employee& employee, std::string& message) = 0;
    virtual bool deactivateEmployee(int employeeId, const std::string& todayDate, std::string& message) = 0;
    virtual std::vector<EmployeeResponse> getAllEmployees() = 0;
    virtual std::vector<EmployeeResponse> getTeamEmployees(int managerUserId) = 0;
    virtual std::optional<Employee> getEmployeeById(int employeeId) = 0;
    virtual bool addSkill(const AddSkillRequest& req, std::string& message) = 0;
    virtual bool updateSkill(const UpdateSkillRequest& req, std::string& message) = 0;
    virtual bool removeSkill(int employeeId, int skillId, std::string& message) = 0;
    virtual std::vector<EmployeeSkillResponse> getSkills(int employeeId) = 0;
};
