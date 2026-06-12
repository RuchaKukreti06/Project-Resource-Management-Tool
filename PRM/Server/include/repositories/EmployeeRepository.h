#pragma once

#include "database/Database.h"
#include "repositories/IEmployeeRepository.h"

class EmployeeRepository : public IEmployeeRepository
{
   public:
    explicit EmployeeRepository(database::Database& database);

    bool createEmployee(const Employee& employee) override;
    bool updateEmployee(const Employee& employee) override;
    Employee getEmployeeById(int id) override;
    Employee getEmployeeByUserId(int userId) override;
    std::vector<Employee> getAllEmployees() override;
    std::vector<Employee> getEmployeesByManager(int managerUserId) override;
    bool setEmployeeActive(int employeeId, bool isActive) override;
    bool setEmployeeStatus(int employeeId, const std::string& status) override;

    int ensureSkill(const std::string& name, const std::string& category) override;
    bool addEmployeeSkill(int employeeId, int skillId, const std::string& proficiencyLevel) override;
    bool updateEmployeeSkill(int employeeId, int skillId,
                             const std::string& proficiencyLevel) override;
    bool removeEmployeeSkill(int employeeId, int skillId) override;
    std::vector<EmployeeSkillView> getEmployeeSkills(int employeeId) override;

   private:
    database::Database& database_;
};
