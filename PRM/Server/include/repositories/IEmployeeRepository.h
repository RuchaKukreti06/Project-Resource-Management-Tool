#pragma once

#include <string>
#include <vector>

#include "models/Employee.h"

struct EmployeeSkillView
{
    int skillId = 0;
    std::string skillName;
    std::string category;
    std::string proficiencyLevel;
};

class IEmployeeRepository
{
   public:
    virtual ~IEmployeeRepository() = default;

    virtual bool createEmployee(const Employee& employee) = 0;
    virtual bool updateEmployee(const Employee& employee) = 0;
    virtual Employee getEmployeeById(int id) = 0;
    virtual Employee getEmployeeByUserId(int userId) = 0;
    virtual std::vector<Employee> getAllEmployees() = 0;
    virtual std::vector<Employee> getEmployeesByManager(int managerUserId) = 0;
    virtual bool setEmployeeActive(int employeeId, bool isActive) = 0;
    virtual bool setEmployeeStatus(int employeeId, const std::string& status) = 0;

    virtual int ensureSkill(const std::string& name, const std::string& category) = 0;
    virtual bool addEmployeeSkill(int employeeId, int skillId, const std::string& proficiencyLevel) = 0;
    virtual bool updateEmployeeSkill(int employeeId, int skillId,
                                     const std::string& proficiencyLevel) = 0;
    virtual bool removeEmployeeSkill(int employeeId, int skillId) = 0;
    virtual std::vector<EmployeeSkillView> getEmployeeSkills(int employeeId) = 0;
};
