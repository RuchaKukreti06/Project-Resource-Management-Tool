#pragma once

#include <string>

class UserValidator
{
public:
    // Validates that username, password, email, and fullName are non-empty,
    // and that role is one of ADMIN / MANAGER / EMPLOYEE.
    bool validateCreate(const std::string& username,
                        const std::string& password,
                        const std::string& role,
                        const std::string& email,
                        const std::string& fullName,
                        std::string& message) const;

    // Validates that username is non-empty.
    bool validateUsername(const std::string& username, std::string& message) const;

    // Validates that newPassword is non-empty.
    bool validateNewPassword(const std::string& newPassword, std::string& message) const;

    // Validates that userId is a positive integer.
    bool validateId(int userId, std::string& message) const;
};
