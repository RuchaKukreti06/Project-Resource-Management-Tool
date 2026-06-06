#include "UserRepository.h"

UserRepository::UserRepository(std::shared_ptr<DatabaseManager> database) : database_(std::move(database)) {}

bool UserRepository::createUser(const User& user)
{
    try
    {
        auto session = database_->getSession();
        auto schema = session.getSchema(database_->getDatabaseName());
        auto usersTable = schema.getTable("users");

        usersTable.insert("username", "password_hash", "role", "status", "force_password_change")
            .values(user.username, user.passwordHash, user.role, user.status, user.forcePasswordChange)
            .execute();

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
        auto session = database_->getSession();
        auto schema = session.getSchema(database_->getDatabaseName());
        auto usersTable = schema.getTable("users");

        auto result = usersTable.select("id", "username", "password_hash", "role", "status", "force_password_change")
            .where("id = :id")
            .bind("id", id)
            .execute();

        if (result.count() > 0)
        {
            auto row = result.fetchOne();
            user.id = row[0].get<int>();
            user.username = row[1].get<std::string>();
            user.passwordHash = row[2].get<std::string>();
            user.role = row[3].get<std::string>();
            user.status = row[4].get<std::string>();
            user.forcePasswordChange = row[5].get<bool>();
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
        auto session = database_->getSession();
        auto schema = session.getSchema(database_->getDatabaseName());
        auto usersTable = schema.getTable("users");

        auto result = usersTable.select("id", "username", "password_hash", "role", "status", "force_password_change")
            .where("username = :username")
            .bind("username", username)
            .execute();

        if (result.count() > 0)
        {
            auto row = result.fetchOne();
            user.id = row[0].get<int>();
            user.username = row[1].get<std::string>();
            user.passwordHash = row[2].get<std::string>();
            user.role = row[3].get<std::string>();
            user.status = row[4].get<std::string>();
            user.forcePasswordChange = row[5].get<bool>();
        }
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to get user by username: {}", error.what());
    }
    return user;
}

std::vector<User> UserRepository::getAllUsers()
{
    std::vector<User> users;
    try
    {
        auto session = database_->getSession();
        auto schema = session.getSchema(database_->getDatabaseName());
        auto usersTable = schema.getTable("users");

        auto result = usersTable.select("id", "username", "password_hash", "role", "status", "force_password_change")
            .execute();

        while (result.next())
        {
            auto row = result.fetchOne();
            User user;
            user.id = row[0].get<int>();
            user.username = row[1].get<std::string>();
            user.passwordHash = row[2].get<std::string>();
            user.role = row[3].get<std::string>();
            user.status = row[4].get<std::string>();
            user.forcePasswordChange = row[5].get<bool>();
            users.push_back(user);
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
        auto session = database_->getSession();
        auto schema = session.getSchema(database_->getDatabaseName());
        auto usersTable = schema.getTable("users");

        usersTable.update()
            .set("username", user.username)
            .set("password_hash", user.passwordHash)
            .set("role", user.role)
            .set("status", user.status)
            .set("force_password_change", user.forcePasswordChange)
            .where("id = :id")
            .bind("id", user.id)
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
        auto session = database_->getSession();
        auto schema = session.getSchema(database_->getDatabaseName());
        auto usersTable = schema.getTable("users");

        usersTable.remove()
            .where("id = :id")
            .bind("id", id)
            .execute();

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
        auto session = database_->getSession();
        auto schema = session.getSchema(database_->getDatabaseName());
        auto usersTable = schema.getTable("users");

        usersTable.update()
            .set("password_hash", passwordHash)
            .where("id = :id")
            .bind("id", id)
            .execute();

        return true;
    }
    catch (const mysqlx::Error& error)
    {
        spdlog::error("Failed to update user password: {}", error.what());
        return false;
    }
}
