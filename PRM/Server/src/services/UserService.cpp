#include "services/UserService.h"
#include "dto/DTOMapper.h"

#include <spdlog/spdlog.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "exceptions/Exceptions.h"

UserService::UserService(std::shared_ptr<IUserRepository> repository,
                         std::shared_ptr<IPasswordHasher> passwordHasher)
    : repository_(std::move(repository)), passwordHasher_(std::move(passwordHasher))
{
}

std::vector<UserResponse> UserService::getAllUsers()
{
    auto users = repository_->getAllUsers();
    return DTOMapper::mapToUserResponse(users);
}

std::optional<UserResponse> UserService::getUserById(int id)
{
    auto user = repository_->getUserById(id);
    if (user.id == 0)
    {
        return std::nullopt;
    }
    return DTOMapper::mapToUserResponse(user);
}

User UserService::getUserByUsername(const std::string& username)
{
    return repository_->getUserByUsername(username);
}

bool UserService::createUser(const UserCreateRequest& req)
{
    std::string message;
    if (!userValidator_.validateCreate(req.username, req.password, req.role, req.email, req.fullName, message))
    {
        throw exceptions::ValidationException(message);
    }

    if (!repository_->getUserByUsername(req.username).username.empty())
    {
        throw exceptions::ConflictException(std::string("Username '") + req.username + "' already exists.");
    }

    if (!repository_->getUserByEmail(req.email).email.empty())
    {
        throw exceptions::ConflictException(std::string("Email '") + req.email + "' is already registered.");
    }

    User user;
    user.username            = req.username;
    user.passwordHash        = passwordHasher_->hashPassword(req.password);
    user.role                = req.role;
    user.email               = req.email;
    user.fullName            = req.fullName;
    user.department          = req.department;
    user.designation         = req.designation;
    user.status              = "ACTIVE";
    user.isActive            = true;
    user.forcePasswordChange = req.forcePasswordChange;

    if (!repository_->createUser(user))
    {
        throw exceptions::DatabaseException("Failed to create user in database.");
    }
    return true;
}

bool UserService::deactivateUser(int id)
{
    if (!repository_->setUserStatus(id, "INACTIVE"))
    {
        throw exceptions::NotFoundException("Failed to deactivate user. User not found.");
    }
    return true;
}

bool UserService::reactivateUser(int id)
{
    if (!repository_->setUserStatus(id, "ACTIVE"))
    {
        throw exceptions::NotFoundException("Failed to reactivate user. User not found.");
    }
    return true;
}

bool UserService::resetPassword(int id, const std::string& newPassword, bool forcePasswordChange)
{
    std::string message;
    if (!userValidator_.validateNewPassword(newPassword, message))
    {
        throw exceptions::ValidationException(message);
    }

    const bool passwordUpdated = repository_->updatePassword(id, passwordHasher_->hashPassword(newPassword));
    if (!passwordUpdated)
    {
        throw exceptions::NotFoundException("Failed to update password. User not found.");
    }

    if (!repository_->setForcePasswordChange(id, forcePasswordChange))
    {
        throw exceptions::DatabaseException("Failed to set force password change flag.");
    }
    return true;
}

bool UserService::assignManager(int userId, int managerId)
{
    std::string message;
    if (!userValidator_.validateId(userId, message))
    {
        throw exceptions::ValidationException(message);
    }

    if (managerId != 0)
    {
        auto manager = repository_->getUserById(managerId);
        if (manager.id == 0)
        {
            throw exceptions::NotFoundException("Manager user not found.");
        }
        if (manager.role != "MANAGER" && manager.role != "ADMIN")
        {
            throw exceptions::ValidationException("User assigned as manager must have MANAGER or ADMIN role.");
        }
    }

    // managerId == 0 means "unassign manager", which the repo handles by setting NULL
    if (!repository_->assignManager(userId, managerId))
    {
        throw exceptions::NotFoundException("Failed to assign manager. User or manager not found.");
    }
    return true;
}
