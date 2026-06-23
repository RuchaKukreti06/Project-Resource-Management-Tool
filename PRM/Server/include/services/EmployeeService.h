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

    void createEmployeeProfile(const EmployeeCreateRequest& req) override;
    void updateEmployeeProfile(const Employee& employee) override;
    void deactivateEmployee(int employeeId, const std::string& todayDate) override;
    std::vector<EmployeeResponse> getAllEmployees() override;
    std::vector<EmployeeResponse> getTeamEmployees(int managerId) override;
    std::optional<Employee> getEmployeeById(int employeeId) override;
    std::optional<Employee> getEmployeeByUserId(int userId) override;

    void addSkill(const AddSkillRequest& req) override;
    void updateSkill(const UpdateSkillRequest& req) override;
    void removeSkill(int employeeId, int skillId) override;
    std::vector<EmployeeSkillResponse> getSkills(int employeeId) override;

   private:
    std::shared_ptr<IEmployeeRepository>  employeeRepository_;
    std::shared_ptr<IUserRepository>      userRepository_;
    std::shared_ptr<IAllocationRepository> allocationRepository_;
    EmployeeValidator                     employeeValidator_;
};
