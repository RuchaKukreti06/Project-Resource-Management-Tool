#pragma once

#include <vector>
#include <optional>
#include <string>
#include "models/Employee.h"
#include "repositories/IEmployeeRepository.h"

class IEmployeeService {
public:
    virtual ~IEmployeeService() = default;
    virtual bool createEmployeeProfile(int userId, const std::string& fullName, const std::string& email,
                                       const std::string& department, const std::string& designation, std::string& message) = 0;
    virtual bool updateEmployeeProfile(const Employee& employee, std::string& message) = 0;
    virtual bool deactivateEmployee(int employeeId, const std::string& todayDate, std::string& message) = 0;
    virtual std::vector<Employee> getAllEmployees() = 0;
    virtual std::vector<Employee> getTeamEmployees(int managerUserId) = 0;
    virtual std::optional<Employee> getEmployeeById(int employeeId) = 0;
    virtual bool addSkill(int employeeId, const std::string& skillName, const std::string& category,
                          const std::string& proficiency, std::string& message) = 0;
    virtual bool updateSkill(int employeeId, int skillId, const std::string& proficiency, std::string& message) = 0;
    virtual bool removeSkill(int employeeId, int skillId, std::string& message) = 0;
    virtual std::vector<EmployeeSkillView> getSkills(int employeeId) = 0;
};
