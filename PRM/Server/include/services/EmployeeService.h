#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "models/Employee.h"
#include "repositories/IAllocationRepository.h"
#include "repositories/IEmployeeRepository.h"
#include "repositories/IUserRepository.h"
#include "services/interfaces/IEmployeeService.h"

class EmployeeService : public IEmployeeService
{
   public:
    EmployeeService(std::shared_ptr<IEmployeeRepository> employeeRepository,
                    std::shared_ptr<IUserRepository> userRepository,
                    std::shared_ptr<IAllocationRepository> allocationRepository);

    bool createEmployeeProfile(int userId, const std::string& fullName, const std::string& email,
                               const std::string& department, const std::string& designation,
                               std::string& message) override;
    bool updateEmployeeProfile(const Employee& employee, std::string& message) override;
    bool deactivateEmployee(int employeeId, const std::string& todayDate,
                            std::string& message) override;
    std::vector<Employee> getAllEmployees() override;
    std::vector<Employee> getTeamEmployees(int managerUserId) override;
    std::optional<Employee> getEmployeeById(int employeeId) override;

    bool addSkill(int employeeId, const std::string& skillName, const std::string& category,
                  const std::string& proficiency, std::string& message) override;
    bool updateSkill(int employeeId, int skillId, const std::string& proficiency,
                     std::string& message) override;
    bool removeSkill(int employeeId, int skillId, std::string& message) override;
    std::vector<EmployeeSkillView> getSkills(int employeeId) override;

   private:
    std::shared_ptr<IEmployeeRepository> employeeRepository_;
    std::shared_ptr<IUserRepository> userRepository_;
    std::shared_ptr<IAllocationRepository> allocationRepository_;
};
