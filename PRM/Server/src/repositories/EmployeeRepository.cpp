#include "repositories/EmployeeRepository.h"

#include <mysqlx/xdevapi.h>
#include <spdlog/spdlog.h>

namespace
{

Employee mapEmployeeRow(const mysqlx::Row& row)
{
    Employee employee;
    employee.id               = row[0].get<int>(); // resources.id
    employee.user_id          = row[1].get<int>(); // users.id
    employee.fullName         = row[2].get<std::string>();
    employee.email            = row[3].get<std::string>();
    employee.department       = !row[4].isNull() ? row[4].get<std::string>() : "";
    employee.designation      = !row[5].isNull() ? row[5].get<std::string>() : "";
    employee.status           = row[6].get<std::string>(); // resources.status
    employee.isActive         = row[7].get<bool>();
    employee.totalUtilisation = row[8].get<int>();          // D7: resources.total_utilisation
    employee.manager_id       = !row[9].isNull() ? row[9].get<int>() : 0;
    return employee;
}

}  // namespace

EmployeeRepository::EmployeeRepository(database::Database& database) : database_(database)
{
}

bool EmployeeRepository::createEmployee(const Employee& employee)
{
    try
    {
        database_.getSchema()
            .getTable("resources")
            .insert("user_id", "status", "total_utilisation")
            .values(employee.user_id, employee.status, 0)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("createEmployee failed: {}", e.what());
        return false;
    }
}

bool EmployeeRepository::updateEmployee(const Employee& employee)
{
    try
    {
        // Resolve user_id from resources table (controller may not pass it)
        int resolvedUserId = employee.user_id;
        if (resolvedUserId == 0)
        {
            auto res = database_.getSchema()
                           .getTable("resources")
                           .select("user_id")
                           .where("id = :id")
                           .bind("id", employee.id)
                           .execute();
            if (auto row = res.fetchOne())
                resolvedUserId = row[0].get<int>();
        }

        if (resolvedUserId == 0)
        {
            spdlog::error("updateEmployee: could not resolve user_id for resource id={}", employee.id);
            return false;
        }

        // Update user profile fields
        database_.getSchema()
            .getTable("users")
            .update()
            .set("full_name", employee.fullName)
            .set("email", employee.email)
            .set("department", employee.department)
            .set("designation", employee.designation)
            .set("is_active", employee.isActive)
            .where("id = :user_id")
            .bind("user_id", resolvedUserId)
            .execute();

        // Update resource status
        database_.getSchema()
            .getTable("resources")
            .update()
            .set("status", employee.status)
            .where("id = :id")
            .bind("id", employee.id)
            .execute();

        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("updateEmployee failed: {}", e.what());
        return false;
    }
}

Employee EmployeeRepository::getEmployeeById(int id)
{
    Employee employee;
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT r.id, r.user_id, u.full_name, u.email, u.department, "
                               "u.designation, r.status, u.is_active, r.total_utilisation, u.manager_id "
                               "FROM resources r "
                               "JOIN users u ON r.user_id = u.id "
                               "WHERE r.id = ?")
                          .bind(id)
                          .execute();

        if (auto row = result.fetchOne())
        {
            employee = mapEmployeeRow(row);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getEmployeeById failed: {}", e.what());
    }
    return employee;
}

Employee EmployeeRepository::getEmployeeByUserId(int userId)
{
    Employee employee;
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT r.id, r.user_id, u.full_name, u.email, u.department, "
                               "u.designation, r.status, u.is_active, r.total_utilisation, u.manager_id "
                               "FROM resources r "
                               "JOIN users u ON r.user_id = u.id "
                               "WHERE r.user_id = ?")
                          .bind(userId)
                          .execute();

        if (auto row = result.fetchOne())
        {
            employee = mapEmployeeRow(row);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getEmployeeByUserId failed: {}", e.what());
    }
    return employee;
}

std::vector<Employee> EmployeeRepository::getAllEmployees()
{
    std::vector<Employee> employees;
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT r.id, r.user_id, u.full_name, u.email, u.department, "
                               "u.designation, r.status, u.is_active, r.total_utilisation, u.manager_id "
                               "FROM resources r "
                               "JOIN users u ON r.user_id = u.id")
                          .execute();

        while (auto row = result.fetchOne())
        {
            employees.push_back(mapEmployeeRow(row));
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getAllEmployees failed: {}", e.what());
    }
    return employees;
}

std::vector<Employee> EmployeeRepository::getEmployeesByManager(int managerUserId)
{
    std::vector<Employee> employees;
    try
    {
        auto result =
            database_.getSession()
                .sql(
                    "SELECT r.id, r.user_id, u.full_name, u.email, u.department, "
                    "u.designation, r.status, u.is_active, r.total_utilisation, u.manager_id "
                    "FROM resources r "
                    "JOIN users u ON r.user_id = u.id "
                    "WHERE u.manager_id = ? AND u.is_active = 1")
                .bind(managerUserId)
                .execute();

        for (auto row : result)
            employees.push_back(mapEmployeeRow(row));
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getEmployeesByManager failed: {}", e.what());
    }
    return employees;
}

bool EmployeeRepository::setEmployeeActive(int employeeId, bool isActive)
{
    try
    {
        // Get user_id first to update the users table
        auto res = database_.getSchema().getTable("resources").select("user_id").where("id = :id").bind("id", employeeId).execute();
        if (auto row = res.fetchOne()) {
            int userId = row[0].get<int>();
            database_.getSchema()
                .getTable("users")
                .update()
                .set("is_active", isActive)
                .where("id = :user_id")
                .bind("user_id", userId)
                .execute();
        }
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("setEmployeeActive failed: {}", e.what());
        return false;
    }
}

bool EmployeeRepository::setEmployeeStatus(int employeeId, const std::string& status)
{
    try
    {
        database_.getSchema()
            .getTable("resources")
            .update()
            .set("status", status)
            .where("id = :id")
            .bind("id", employeeId)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("setEmployeeStatus failed: {}", e.what());
        return false;
    }
}

int EmployeeRepository::ensureSkill(const std::string& name, const std::string& category)
{
    try
    {
        auto existing = database_.getSchema()
                            .getTable("skills")
                            .select("id")
                            .where("skill_name = :name")
                            .bind("name", name)
                            .execute();

        if (auto row = existing.fetchOne())
        {
            return row.get(0);
        }

        database_.getSchema()
            .getTable("skills")
            .insert("skill_name", "category")
            .values(name, category)
            .execute();

        auto inserted = database_.getSchema()
                            .getTable("skills")
                            .select("id")
                            .where("skill_name = :name")
                            .bind("name", name)
                            .execute();
        if (auto row = inserted.fetchOne())
        {
            return row.get(0);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("ensureSkill failed: {}", e.what());
    }

    return 0;
}

bool EmployeeRepository::addEmployeeSkill(int employeeId, int skillId,
                                          const std::string& proficiencyLevel)
{
    try
    {
        database_.getSchema()
            .getTable("resource_skills")
            .insert("resource_id", "skill_id", "proficiency_level")
            .values(employeeId, skillId, proficiencyLevel)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("addEmployeeSkill failed: {}", e.what());
        return false;
    }
}

bool EmployeeRepository::updateEmployeeSkill(int employeeId, int skillId,
                                             const std::string& proficiencyLevel)
{
    try
    {
        database_.getSchema()
            .getTable("resource_skills")
            .update()
            .set("proficiency_level", proficiencyLevel)
            .where("resource_id = :employee_id AND skill_id = :skill_id")
            .bind("employee_id", employeeId)
            .bind("skill_id", skillId)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("updateEmployeeSkill failed: {}", e.what());
        return false;
    }
}

bool EmployeeRepository::removeEmployeeSkill(int employeeId, int skillId)
{
    try
    {
        database_.getSchema()
            .getTable("resource_skills")
            .remove()
            .where("resource_id = :employee_id AND skill_id = :skill_id")
            .bind("employee_id", employeeId)
            .bind("skill_id", skillId)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("removeEmployeeSkill failed: {}", e.what());
        return false;
    }
}

std::vector<EmployeeSkillView> EmployeeRepository::getEmployeeSkills(int employeeId)
{
    std::vector<EmployeeSkillView> skills;
    try
    {
        auto result = database_.getSession()
                          .sql(
                              "SELECT s.id, s.skill_name, s.category, es.proficiency_level "
                              "FROM resource_skills es "
                              "JOIN skills s ON s.id = es.skill_id "
                              "WHERE es.resource_id = ?")
                          .bind(employeeId)
                          .execute();

        for (auto row : result)
        {
            EmployeeSkillView view;
            view.skillId = row[0].get<int>();
            view.skillName = row[1].get<std::string>();
            view.category = row[2].get<std::string>();
            view.proficiencyLevel = row[3].get<std::string>();
            skills.push_back(view);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getEmployeeSkills failed: {}", e.what());
    }
    return skills;
}
