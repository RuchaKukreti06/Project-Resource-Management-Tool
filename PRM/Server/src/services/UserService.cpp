#include "services/UserService.h"

#include <spdlog/spdlog.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

UserService::UserService(std::shared_ptr<IUserRepository> repository, std::shared_ptr<IPasswordHasher> passwordHasher)
    : repository_(std::move(repository)), passwordHasher_(std::move(passwordHasher))
{
}

std::vector<User> UserService::getAllUsers()
{
    return repository_->getAllUsers();
}

std::optional<User> UserService::getUserById(int id)
{
    User user = repository_->getUserById(id);
    if (user.id == 0)
    {
        return std::nullopt;
    }
    return user;
}

User UserService::getUserByUsername(const std::string& username)
{
    return repository_->getUserByUsername(username);
}

bool UserService::createUser(const std::string& username, const std::string& password,
                             const std::string& role, const std::string& email,
                             const std::string& fullName, const std::string& department,
                             const std::string& designation, bool forcePasswordChange)
{
    static const std::unordered_set<std::string> validRoles = {"ADMIN", "MANAGER", "EMPLOYEE"};

    if (username.empty() || password.empty() || email.empty() || fullName.empty() ||
        validRoles.find(role) == validRoles.end())
    {
        throw std::runtime_error("Invalid user input. Check roles and mandatory fields.");
    }

    if (!repository_->getUserByUsername(username).username.empty())
    {
        throw std::runtime_error(std::string("Username '") + username + "' already exists.");
    }

    if (!repository_->getUserByEmail(email).email.empty())
    {
        throw std::runtime_error(std::string("Email '") + email + "' is already registered.");
    }

    User user;
    user.username            = username;
    user.passwordHash        = passwordHasher_->hashPassword(password);
    user.role                = role;
    user.email               = email;
    user.fullName            = fullName;
    user.department          = department;
    user.designation         = designation;
    user.status              = "ACTIVE";
    user.isActive            = true;
    user.forcePasswordChange = forcePasswordChange;

    return repository_->createUser(user);
}

bool UserService::deactivateUser(int id)
{
    return repository_->setUserStatus(id, "INACTIVE");
}

bool UserService::reactivateUser(int id)
{
    return repository_->setUserStatus(id, "ACTIVE");
}

bool UserService::resetPassword(int id, const std::string& newPassword, bool forcePasswordChange)
{
    if (newPassword.empty())
    {
        return false;
    }

    const bool passwordUpdated = repository_->updatePassword(id, passwordHasher_->hashPassword(newPassword));
    if (!passwordUpdated)
    {
        return false;
    }

    return repository_->setForcePasswordChange(id, forcePasswordChange);
}



bool UserService::assignManager(int userId, int managerId)
{
    if (userId <= 0)
        return false;
    // managerId == 0 means "unassign manager", which the repo handles by setting NULL
    return repository_->assignManager(userId, managerId);
}
