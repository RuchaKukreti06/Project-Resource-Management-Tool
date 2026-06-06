#pragma once

#include <memory>
#include <string>
#include <vector>
#include "database/Database.h"
#include "models/User.h"

class UserRepository
{
public:
    explicit UserRepository(std::shared_ptr<DatabaseManager> database);
    bool createUser(const User& user);
    User getUserById(int id);
    User getUserByUsername(const std::string& username);
    std::vector<User> getAllUsers();
    bool updateUser(const User& user);
    bool deleteUser(int id);
    bool updatePassword(int id,const std::string& passwordHash);
};