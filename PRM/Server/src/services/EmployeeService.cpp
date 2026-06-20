#include "services/EmployeeService.h"

#include "utils/DateUtils.h"

EmployeeService::EmployeeService(std::shared_ptr<IEmployeeRepository> employeeRepository,
                                 std::shared_ptr<IUserRepository> userRepository,
                                 std::shared_ptr<IAllocationRepository> allocationRepository)
    : employeeRepository_(std::move(employeeRepository)),
      userRepository_(std::move(userRepository)),
      allocationRepository_(std::move(allocationRepository))
{
}

bool EmployeeService::createEmployeeProfile(int userId, const std::string& fullName,
                                            const std::string& email,
                                            const std::string& department,
                                            const std::string& designation,
                                            std::string& message)
{
    const User user = userRepository_->getUserById(userId);
    if (user.id == 0)
    {
        message = "User ID not found.";
        return false;
    }

    if (user.role != "EMPLOYEE" && user.role != "MANAGER")
    {
        message = "Target user must be EMPLOYEE or MANAGER.";
        return false;
    }

    if (employeeRepository_->getEmployeeByUserId(userId).id != 0)
    {
        message = "Profile already linked to this user ID.";
        return false;
    }

    Employee employee;
    employee.user_id     = userId;
    employee.fullName    = fullName;
    employee.email       = email;
    employee.department  = department;
    employee.designation = designation;
    employee.status      = "BENCH";
    employee.isActive    = true;

    const bool ok = employeeRepository_->createEmployee(employee);
    message = ok ? "Profile created." : "Failed to create profile.";
    return ok;
}

bool EmployeeService::updateEmployeeProfile(const Employee& employee, std::string& message)
{
    if (!employeeValidator_.validateId(employee.id, message))
    {
        return false;
    }

    const bool ok = employeeRepository_->updateEmployee(employee);
    message = ok ? "Employee updated." : "Failed to update employee.";
    return ok;
}

bool EmployeeService::deactivateEmployee(int employeeId, const std::string& todayDate,
                                         std::string& message)
{
    const auto employee = employeeRepository_->getEmployeeById(employeeId);
    if (employee.id == 0)
    {
        message = "Employee not found.";
        return false;
    }

    const std::string effectiveDate = todayDate.empty() ? utils::currentDateIso() : todayDate;

    if (!allocationRepository_->endActiveAllocationsByEmployee(employeeId, effectiveDate))
    {
        message = "Failed to end active allocations.";
        return false;
    }

    if (!employeeRepository_->setEmployeeActive(employeeId, false) ||
        !employeeRepository_->setEmployeeStatus(employeeId, "BENCH"))
    {
        message = "Failed to deactivate employee profile.";
        return false;
    }

    if (!userRepository_->setUserStatus(employee.user_id, "INACTIVE"))
    {
        message = "Employee deactivated but linked user status update failed.";
        return false;
    }

    message = "Employee deactivated. Active allocations ended and login blocked.";
    return true;
}

std::vector<Employee> EmployeeService::getAllEmployees()
{
    return employeeRepository_->getAllEmployees();
}

std::vector<Employee> EmployeeService::getTeamEmployees(int managerUserId)
{
    return employeeRepository_->getEmployeesByManager(managerUserId);
}

std::optional<Employee> EmployeeService::getEmployeeById(int employeeId)
{
    auto employee = employeeRepository_->getEmployeeById(employeeId);
    if (employee.id == 0)
    {
        return std::nullopt;
    }
    return employee;
}

bool EmployeeService::addSkill(int employeeId, const std::string& skillName,
                               const std::string& category, const std::string& proficiency,
                               std::string& message)
{
    if (!employeeValidator_.validateSkill(category, proficiency, message))
    {
        return false;
    }

    const int skillId = employeeRepository_->ensureSkill(skillName, category);
    if (skillId <= 0)
    {
        message = "Could not create/find skill.";
        return false;
    }

    const bool ok = employeeRepository_->addEmployeeSkill(employeeId, skillId, proficiency);
    message = ok ? "Skill added." : "Failed to add skill.";
    return ok;
}

bool EmployeeService::updateSkill(int employeeId, int skillId, const std::string& proficiency,
                                  std::string& message)
{
    const bool ok = employeeRepository_->updateEmployeeSkill(employeeId, skillId, proficiency);
    message = ok ? "Skill updated." : "Failed to update skill.";
    return ok;
}

bool EmployeeService::removeSkill(int employeeId, int skillId, std::string& message)
{
    const bool ok = employeeRepository_->removeEmployeeSkill(employeeId, skillId);
    message = ok ? "Skill removed." : "Failed to remove skill.";
    return ok;
}

std::vector<EmployeeSkillView> EmployeeService::getSkills(int employeeId)
{
    return employeeRepository_->getEmployeeSkills(employeeId);
}
