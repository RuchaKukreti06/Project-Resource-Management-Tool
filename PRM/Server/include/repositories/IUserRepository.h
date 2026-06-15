#pragma once

#include <string>
#include <vector>

#include "User.h"

class IUserRepository
{
   public:
    virtual ~IUserRepository() = default;

    virtual bool createUser(const User& user) = 0;
    virtual User getUserById(int id) = 0;
    virtual User getUserByUsername(const std::string& username) = 0;
    virtual User getUserByEmail(const std::string& email) = 0;
    virtual std::vector<User> getAllUsers() = 0;
    virtual bool updateUser(const User& user) = 0;
    virtual bool deleteUser(int id) = 0;
    virtual bool updatePassword(int id, const std::string& passwordHash) = 0;
    virtual bool setUserStatus(int id, const std::string& status) = 0;
    virtual bool setForcePasswordChange(int id, bool forcePasswordChange) = 0;
    virtual bool assignManager(int userId, int managerId) = 0;
};
