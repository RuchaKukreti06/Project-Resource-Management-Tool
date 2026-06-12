#include "UserRepository.h"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace
{

// Maps a row from: id, username, password_hash, role_name, is_active,
//                  force_password_change, email, full_name, department,
//                  designation, role_id, manager_id, created_at, updated_at
User mapUserRow(const mysqlx::Row& row)
{
    User user;
    user.id                  = row[0].get<int>();
    user.username            = row[1].get<std::string>();
    user.passwordHash        = row[2].get<std::string>();
    user.role                = row[3].get<std::string>();          // role name from JOIN
    user.isActive            = row[4].get<bool>();
    user.status              = user.isActive ? "ACTIVE" : "INACTIVE";
    user.forcePasswordChange = row[5].get<bool>();
    user.email               = !row[6].isNull() ? row[6].get<std::string>() : "";
    user.fullName            = !row[7].isNull() ? row[7].get<std::string>() : "";
    user.department          = !row[8].isNull() ? row[8].get<std::string>() : "";
    user.designation         = !row[9].isNull() ? row[9].get<std::string>() : "";
    user.roleId              = row[10].get<int>();
    user.managerId           = !row[11].isNull() ? row[11].get<int>() : 0;
    user.createdAt           = !row[12].isNull() ? row[12].get<std::string>() : "";
    user.updatedAt           = !row[13].isNull() ? row[13].get<std::string>() : "";
    return user;
}

// Base SELECT with JOIN used by all fetch methods
static const char* kSelectUserSQL =
    "SELECT u.id, u.username, u.password_hash, r.name, u.is_active, "
    "u.force_password_change, u.email, u.full_name, u.department, u.designation, "
    "u.role_id, u.manager_id, DATE_FORMAT(u.created_at, '%Y-%m-%d %T'), DATE_FORMAT(u.updated_at, '%Y-%m-%d %T') "
    "FROM users u JOIN roles r ON r.id = u.role_id";

}  // namespace

UserRepository::UserRepository(database::Database& database) : database_(database)
{
}

bool UserRepository::createUser(const User& user)
{
    try
    {
        // Look up role_id from role name
        auto roleResult = database_.getSession()
                              .sql("SELECT id FROM roles WHERE name = ?")
                              .bind(user.role)
                              .execute();

        auto roleRow = roleResult.fetchOne();
        if (!roleRow)
        {
            spdlog::error("Failed to create user: unknown role '{}'", user.role);
            return false;
        }
        int roleId = roleRow[0].get<int>();

        database_.getSession()
            .sql("INSERT INTO users (username, password_hash, role_id, email, full_name, "
                 "department, designation, force_password_change, is_active) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)")
            .bind(user.username)
            .bind(user.passwordHash)
            .bind(roleId)
            .bind(user.email)
            .bind(user.fullName)
            .bind(user.department)
            .bind(user.designation)
            .bind(user.forcePasswordChange)
            .bind(user.isActive)
            .execute();

        // If the user is an EMPLOYEE, they also need a record in the `resources` table
        if (user.role == "EMPLOYEE")
        {
            database_.getSession()
                .sql("INSERT INTO resources (user_id, status, total_utilisation) "
                     "VALUES (LAST_INSERT_ID(), 'BENCH', 0)")
                .execute();
        }

        return true;
    }

    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to create user: {}", error.what());
        return false;
    }
}

User UserRepository::getUserById(int id)
{
    User user;
    try
    {
        std::string sql = std::string(kSelectUserSQL) + " WHERE u.id = ?";
        auto result     = database_.getSession().sql(sql).bind(id).execute();

        if (auto row = result.fetchOne())
        {
            user = mapUserRow(row);
        }
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to get user by ID: {}", error.what());
    }
    return user;
}

User UserRepository::getUserByUsername(const std::string& username)
{
    User user;
    try
    {
        std::string sql = std::string(kSelectUserSQL) + " WHERE u.username = ?";
        auto result     = database_.getSession().sql(sql).bind(username).execute();

        if (auto row = result.fetchOne())
        {
            user = mapUserRow(row);
        }
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to get user by username: {}", error.what());
    }
    return user;
}

User UserRepository::getUserByEmail(const std::string& email)
{
    User user;
    try
    {
        std::string sql = std::string(kSelectUserSQL) + " WHERE u.email = ?";
        auto result     = database_.getSession().sql(sql).bind(email).execute();

        if (auto row = result.fetchOne())
        {
            user = mapUserRow(row);
        }
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to get user by email: {}", error.what());
    }
    return user;
}

std::vector<User> UserRepository::getAllUsers()
{
    std::vector<User> users;
    try
    {
        auto result = database_.getSession().sql(kSelectUserSQL).execute();

        for (auto row : result)
        {
            users.push_back(mapUserRow(row));
        }
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to get all users: {}", error.what());
    }
    return users;
}

bool UserRepository::updateUser(const User& user)
{
    try
    {
        // Look up role_id from role name
        int roleId = user.roleId;
        if (roleId == 0 && !user.role.empty())
        {
            auto roleResult = database_.getSession()
                                  .sql("SELECT id FROM roles WHERE name = ?")
                                  .bind(user.role)
                                  .execute();
            if (auto row = roleResult.fetchOne())
            {
                roleId = row[0].get<int>();
            }
        }

        database_.getSession()
            .sql("UPDATE users SET username = ?, password_hash = ?, role_id = ?, email = ?, "
                 "full_name = ?, department = ?, designation = ?, is_active = ?, "
                 "force_password_change = ? WHERE id = ?")
            .bind(user.username)
            .bind(user.passwordHash)
            .bind(roleId)
            .bind(user.email)
            .bind(user.fullName)
            .bind(user.department)
            .bind(user.designation)
            .bind(user.isActive)
            .bind(user.forcePasswordChange)
            .bind(user.id)
            .execute();

        return true;
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to update user: {}", error.what());
        return false;
    }
}

bool UserRepository::deleteUser(int id)
{
    try
    {
        database_.getSession().sql("DELETE FROM users WHERE id = ?").bind(id).execute();
        return true;
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to delete user: {}", error.what());
        return false;
    }
}

bool UserRepository::updatePassword(int id, const std::string& passwordHash)
{
    try
    {
        database_.getSession()
            .sql("UPDATE users SET password_hash = ? WHERE id = ?")
            .bind(passwordHash)
            .bind(id)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to update user password: {}", error.what());
        return false;
    }
}

bool UserRepository::setUserStatus(int id, const std::string& status)
{
    try
    {
        // Schema uses is_active BOOLEAN; map "ACTIVE" → true, anything else → false
        bool isActive = (status == "ACTIVE");
        database_.getSession()
            .sql("UPDATE users SET is_active = ? WHERE id = ?")
            .bind(isActive)
            .bind(id)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to update user status: {}", error.what());
        return false;
    }
}

bool UserRepository::setForcePasswordChange(int id, bool forcePasswordChange)
{
    try
    {
        database_.getSession()
            .sql("UPDATE users SET force_password_change = ? WHERE id = ?")
            .bind(forcePasswordChange)
            .bind(id)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to update force_password_change: {}", error.what());
        return false;
    }
}
bool UserRepository::assignManager(int userId, int managerId)
{
    try
    {
        // Allow clearing the manager by passing managerId = 0
        if (managerId == 0)
        {
            database_.getSession()
                .sql("UPDATE users SET manager_id = NULL WHERE id = ?")
                .bind(userId)
                .execute();
        }
        else
        {
            database_.getSession()
                .sql("UPDATE users SET manager_id = ? WHERE id = ?")
                .bind(managerId)
                .bind(userId)
                .execute();
        }
        return true;
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to assign manager: {}", error.what());
        return false;
    }
}

