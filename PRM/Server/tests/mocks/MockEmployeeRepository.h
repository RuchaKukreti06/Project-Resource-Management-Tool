#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Employee.h"
#include "EmployeeSkills.h"
#include "repositories/IEmployeeRepository.h"

class MockEmployeeRepository : public IEmployeeRepository
{
   public:
    Employee employee;
    std::unordered_map<int, Employee> employees;
    std::unordered_map<int, std::vector<EmployeeSkillView>> skills;

    bool createEmployee(const Employee& e) override
    {
        employees[e.id] = e;
        return true;
    }

    bool updateEmployee(const Employee& e) override
    {
        employees[e.id] = e;
        return true;
    }

    Employee getEmployeeById(int id) override
    {
        auto it = employees.find(id);
        return it != employees.end() ? it->second : Employee{};
    }

    Employee getEmployeeByUserId(int userId) override
    {
        for (auto& [id, emp] : employees)
            if (emp.user_id == userId) return emp;
        return Employee{};
    }

    std::vector<Employee> getAllEmployees() override
    {
        std::vector<Employee> result;
        for (auto& [id, emp] : employees) result.push_back(emp);
        return result;
    }

    std::vector<Employee> getEmployeesByManager(int) override
    {
        std::vector<Employee> result;
        for (auto& [id, emp] : employees) result.push_back(emp);
        return result;
    }

    bool setEmployeeActive(int id, bool isActive) override
    {
        employees[id].isActive = isActive;
        return true;
    }

    bool setEmployeeStatusAndUtilization(int id, const std::string& status, int utilization) override
    {
        employees[id].status = status;
        employees[id].totalUtilisation = utilization;
        return true;
    }

    int ensureSkill(const std::string&, const std::string&) override
    {
        return 1;
    }

    bool addEmployeeSkill(int employeeId, int skillId, const std::string& proficiencyLevel) override
    {
        EmployeeSkillView skill;
        skill.skillId = skillId;
        skill.proficiencyLevel = proficiencyLevel;
        skills[employeeId].push_back(skill);
        return true;
    }

    bool updateEmployeeSkill(int, int, const std::string&) override
    {
        return true;
    }
    bool removeEmployeeSkill(int, int) override
    {
        return true;
    }

    std::vector<EmployeeSkillView> getEmployeeSkills(int employeeId) override
    {
        return skills[employeeId];
    }
};
