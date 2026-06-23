#include "services/EmployeeService.h"
#include "dto/DTOMapper.h"
#include "utils/DateUtils.h"
#include "exceptions/Exceptions.h"

EmployeeService::EmployeeService(std::shared_ptr<IEmployeeRepository> employeeRepository,
                                 std::shared_ptr<IUserRepository> userRepository,
                                 std::shared_ptr<IAllocationRepository> allocationRepository)
    : employeeRepository_(std::move(employeeRepository)),
      userRepository_(std::move(userRepository)),
      allocationRepository_(std::move(allocationRepository))
{
}

void EmployeeService::createEmployeeProfile(const EmployeeCreateRequest& req)
{
    const User user = userRepository_->getUserById(req.userId);
    if (user.id == 0)
    {
        throw exceptions::NotFoundException("User ID not found.");
    }

    if (user.status != "ACTIVE")
    {
        throw exceptions::ValidationException("Target user must be ACTIVE.");
    }

    if (user.role != "EMPLOYEE" && user.role != "MANAGER")
    {
        throw exceptions::ValidationException("Target user must be EMPLOYEE or MANAGER.");
    }

    if (employeeRepository_->getEmployeeByUserId(req.userId).id != 0)
    {
        throw exceptions::ConflictException("Profile already linked to this user ID.");
    }

    Employee employee;
    employee.user_id     = req.userId;
    employee.fullName    = req.fullName;
    employee.email       = req.email;
    employee.department  = req.department;
    employee.designation = req.designation;
    employee.status      = "BENCH";
    employee.isActive    = true;

    if (!employeeRepository_->createEmployee(employee))
    {
        throw exceptions::DatabaseException("Failed to create profile.");
    }
}

void EmployeeService::updateEmployeeProfile(const Employee& employee)
{
    std::string message;
    if (!employeeValidator_.validateId(employee.id, message))
    {
        throw exceptions::ValidationException(message);
    }

    if (!employeeRepository_->updateEmployee(employee))
    {
        throw exceptions::DatabaseException("Failed to update employee.");
    }
}

void EmployeeService::deactivateEmployee(int employeeId, const std::string& todayDate)
{
    const auto employee = employeeRepository_->getEmployeeById(employeeId);
    if (employee.id == 0)
    {
        throw exceptions::NotFoundException("Employee not found.");
    }

    const std::string effectiveDate = todayDate.empty() ? utils::currentDateIso() : todayDate;

    if (!allocationRepository_->endActiveAllocationsByEmployee(employeeId, effectiveDate))
    {
        throw exceptions::DatabaseException("Failed to end active allocations.");
    }

    if (!employeeRepository_->setEmployeeActive(employeeId, false) ||
        !employeeRepository_->setEmployeeStatus(employeeId, "BENCH"))
    {
        throw exceptions::DatabaseException("Failed to deactivate employee profile.");
    }

    if (!userRepository_->setUserStatus(employee.user_id, "INACTIVE"))
    {
        throw exceptions::DatabaseException("Employee deactivated but linked user status update failed.");
    }
}

std::vector<EmployeeResponse> EmployeeService::getAllEmployees()
{
    return DTOMapper::mapToEmployeeResponse(employeeRepository_->getAllEmployees());
}

std::vector<EmployeeResponse> EmployeeService::getTeamEmployees(int managerUserId)
{
    return DTOMapper::mapToEmployeeResponse(employeeRepository_->getEmployeesByManager(managerUserId));
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

std::optional<Employee> EmployeeService::getEmployeeByUserId(int userId)
{
    auto employee = employeeRepository_->getEmployeeByUserId(userId);
    if (employee.id == 0)
    {
        return std::nullopt;
    }
    return employee;
}

void EmployeeService::addSkill(const AddSkillRequest& req)
{
    std::string message;
    if (!employeeValidator_.validateSkill(req.category, req.proficiency, message))
    {
        throw exceptions::ValidationException(message);
    }

    auto existingSkills = getSkills(req.employeeId);
    for (const auto& skill : existingSkills)
    {
        if (skill.skillName == req.skillName && skill.category == req.category)
        {
            throw exceptions::ConflictException("Employee already has this skill.");
        }
    }

    const int skillId = employeeRepository_->ensureSkill(req.skillName, req.category);
    if (skillId <= 0)
    {
        throw exceptions::DatabaseException("Could not create/find skill.");
    }

    if (!employeeRepository_->addEmployeeSkill(req.employeeId, skillId, req.proficiency))
    {
        throw exceptions::DatabaseException("Failed to add skill.");
    }
}

void EmployeeService::updateSkill(const UpdateSkillRequest& req)
{
    if (!employeeRepository_->updateEmployeeSkill(req.employeeId, req.skillId, req.proficiency))
    {
        throw exceptions::DatabaseException("Failed to update skill.");
    }
}

void EmployeeService::removeSkill(int employeeId, int skillId)
{
    if (!employeeRepository_->removeEmployeeSkill(employeeId, skillId))
    {
        throw exceptions::DatabaseException("Failed to remove skill.");
    }
}

std::vector<EmployeeSkillResponse> EmployeeService::getSkills(int employeeId)
{
    auto skills = employeeRepository_->getEmployeeSkills(employeeId);
    return DTOMapper::mapToEmployeeSkillResponse(skills);
}
