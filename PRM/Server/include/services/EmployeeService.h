#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "models/Employee.h"
#include "repositories/IAllocationRepository.h"
#include "repositories/IEmployeeRepository.h"
#include "repositories/IUserRepository.h"

class EmployeeService
{
   public:
    EmployeeService(std::shared_ptr<IEmployeeRepository> employeeRepository,
                    std::shared_ptr<IUserRepository> userRepository,
                    std::shared_ptr<IAllocationRepository> allocationRepository);

    bool createEmployeeProfile(int userId, const std::string& fullName, const std::string& email,
                               const std::string& department, const std::string& designation,
                               std::string& message);
    bool updateEmployeeProfile(const Employee& employee, std::string& message);
    bool deactivateEmployee(int employeeId, const std::string& todayDate, std::string& message);
    std::vector<Employee> getAllEmployees();
    std::vector<Employee> getTeamEmployees(int managerUserId);
    std::optional<Employee> getEmployeeById(int employeeId);

    bool addSkill(int employeeId, const std::string& skillName, const std::string& category,
                  const std::string& proficiency, std::string& message);
    bool updateSkill(int employeeId, int skillId, const std::string& proficiency,
                     std::string& message);
    bool removeSkill(int employeeId, int skillId, std::string& message);
    std::vector<EmployeeSkillView> getSkills(int employeeId);

   private:
    std::shared_ptr<IEmployeeRepository> employeeRepository_;
    std::shared_ptr<IUserRepository> userRepository_;
    std::shared_ptr<IAllocationRepository> allocationRepository_;
};
