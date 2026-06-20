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
#include "validators/EmployeeValidator.h"

class EmployeeService : public IEmployeeService
{
   public:
    EmployeeService(std::shared_ptr<IEmployeeRepository> employeeRepository,
                    std::shared_ptr<IUserRepository> userRepository,
                    std::shared_ptr<IAllocationRepository> allocationRepository);

    bool createEmployeeProfile(const EmployeeCreateRequest& req, std::string& message) override;
    bool updateEmployeeProfile(const Employee& employee, std::string& message) override;
    bool deactivateEmployee(int employeeId, const std::string& todayDate,
                            std::string& message) override;
    std::vector<EmployeeResponse> getAllEmployees() override;
    std::vector<EmployeeResponse> getTeamEmployees(int managerId) override;
    std::optional<Employee> getEmployeeById(int employeeId) override;

    bool addSkill(const AddSkillRequest& req, std::string& message) override;
    bool updateSkill(const UpdateSkillRequest& req, std::string& message) override;
    bool removeSkill(int employeeId, int skillId, std::string& message) override;
    std::vector<EmployeeSkillResponse> getSkills(int employeeId) override;

   private:
    std::shared_ptr<IEmployeeRepository>  employeeRepository_;
    std::shared_ptr<IUserRepository>      userRepository_;
    std::shared_ptr<IAllocationRepository> allocationRepository_;
    EmployeeValidator                     employeeValidator_;
};
