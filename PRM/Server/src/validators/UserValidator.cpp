#include "validators/UserValidator.h"

#include <unordered_set>

bool UserValidator::validateCreate(const std::string& username,
                                   const std::string& password,
                                   const std::string& role,
                                   const std::string& email,
                                   const std::string& fullName,
                                   std::string& message) const
{
    static const std::unordered_set<std::string> validRoles = {"ADMIN", "MANAGER", "EMPLOYEE"};

    if (username.empty() || password.empty() || email.empty() || fullName.empty() ||
        validRoles.find(role) == validRoles.end())
    {
        message = "Invalid user input. Check roles and mandatory fields.";
        return false;
    }

    bool hasDigit = false, hasSpecial = false;
    for (char c : password)
    {
        if (std::isdigit(c)) hasDigit = true;
        if (std::ispunct(c)) hasSpecial = true;
    }
    if (password.length() < 8 || !hasDigit || !hasSpecial)
    {
        message = "Password must be at least 8 characters long, contain a digit and a special character.";
        return false;
    }

    return true;
}

bool UserValidator::validateUsername(const std::string& username, std::string& message) const
{
    if (username.empty())
    {
        message = "Username is required.";
        return false;
    }
    return true;
}

bool UserValidator::validateNewPassword(const std::string& newPassword, std::string& message) const
{
    if (newPassword.empty())
    {
        message = "Password is required.";
        return false;
    }

    bool hasDigit = false, hasSpecial = false;
    for (char c : newPassword)
    {
        if (std::isdigit(c)) hasDigit = true;
        if (std::ispunct(c)) hasSpecial = true;
    }
    if (newPassword.length() < 8 || !hasDigit || !hasSpecial)
    {
        message = "Password must be at least 8 characters long, contain a digit and a special character.";
        return false;
    }

    return true;
}

bool UserValidator::validateId(int userId, std::string& message) const
{
    if (userId <= 0)
    {
        message = "Invalid user id.";
        return false;
    }
    return true;
}
