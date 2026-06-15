#include "services/UserService.h"

#include <openssl/evp.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

#include "database/Database.h"
#include "repositories/UserRepository.h"

UserService::UserService(std::shared_ptr<IUserRepository> repository)
    : repository_(std::move(repository))
{
}

IUserRepository& UserService::repository()
{
    if (!repository_)
    {
        repository_ = std::make_shared<UserRepository>(database::Database::instance());
    }
    return *repository_;
}

std::vector<User> UserService::getAllUsers()
{
    return repository().getAllUsers();
}

std::optional<User> UserService::getUserById(int id)
{
    User user = repository().getUserById(id);
    if (user.id == 0)
    {
        return std::nullopt;
    }
    return user;
}

User UserService::getUserByUsername(const std::string& username)
{
    return repository().getUserByUsername(username);
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

    if (!repository().getUserByUsername(username).username.empty())
    {
        throw std::runtime_error(std::string("Username '") + username + "' already exists.");
    }

    if (!repository().getUserByEmail(email).email.empty())
    {
        throw std::runtime_error(std::string("Email '") + email + "' is already registered.");
    }

    User user;
    user.username            = username;
    user.passwordHash        = hashPassword(password);
    user.role                = role;
    user.email               = email;
    user.fullName            = fullName;
    user.department          = department;
    user.designation         = designation;
    user.status              = "ACTIVE";
    user.isActive            = true;
    user.forcePasswordChange = forcePasswordChange;

    return repository().createUser(user);
}

bool UserService::deactivateUser(int id)
{
    return repository().setUserStatus(id, "INACTIVE");
}

bool UserService::reactivateUser(int id)
{
    return repository().setUserStatus(id, "ACTIVE");
}

bool UserService::resetPassword(int id, const std::string& newPassword, bool forcePasswordChange)
{
    if (newPassword.empty())
    {
        return false;
    }

    const bool passwordUpdated = repository().updatePassword(id, hashPassword(newPassword));
    if (!passwordUpdated)
    {
        return false;
    }

    return repository().setForcePasswordChange(id, forcePasswordChange);
}

std::string UserService::hashPassword(const std::string& password) const
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLength = 0;
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == nullptr)
    {
        throw std::runtime_error("Failed to create OpenSSL message digest context");
    }

    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(context, password.data(), password.size()) != 1 ||
        EVP_DigestFinal_ex(context, digest, &digestLength) != 1)
    {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to compute password hash");
    }
    EVP_MD_CTX_free(context);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < digestLength; ++i)
    {
        oss << std::setw(2) << static_cast<int>(digest[i]);
    }
    return oss.str();
}

bool UserService::assignManager(int userId, int managerId)
{
    if (userId <= 0)
        return false;
    // managerId == 0 means "unassign manager", which the repo handles by setting NULL
    return repository().assignManager(userId, managerId);
}
