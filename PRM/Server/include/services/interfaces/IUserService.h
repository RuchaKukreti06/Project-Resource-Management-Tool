#pragma once

#include <vector>
#include <optional>
#include <string>
#include "models/User.h"

class IUserService {
public:
    virtual ~IUserService() = default;
    virtual std::vector<User> getAllUsers() = 0;
    virtual std::optional<User> getUserById(int id) = 0;
    virtual User getUserByUsername(const std::string& username) = 0;
    virtual bool createUser(const std::string& username, const std::string& password,
                            const std::string& role, const std::string& email,
                            const std::string& fullName, const std::string& department = "",
                            const std::string& designation = "", bool forcePasswordChange = true) = 0;
    virtual bool deactivateUser(int id) = 0;
    virtual bool reactivateUser(int id) = 0;
    virtual bool resetPassword(int id, const std::string& newPassword, bool forcePasswordChange = true) = 0;
    virtual bool assignManager(int userId, int managerId) = 0;
};
