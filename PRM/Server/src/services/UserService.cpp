#include "services/UserService.h"
#include "dto/DTOMapper.h"

#include <spdlog/spdlog.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

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
        throw std::runtime_error(message);
    }

    if (!repository_->getUserByUsername(req.username).username.empty())
    {
        throw std::runtime_error(std::string("Username '") + req.username + "' already exists.");
    }

    if (!repository_->getUserByEmail(req.email).email.empty())
    {
        throw std::runtime_error(std::string("Email '") + req.email + "' is already registered.");
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
    std::string message;
    if (!userValidator_.validateNewPassword(newPassword, message))
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
    std::string message;
    if (!userValidator_.validateId(userId, message))
    {
        return false;
    }
    // managerId == 0 means "unassign manager", which the repo handles by setting NULL
    return repository_->assignManager(userId, managerId);
}
