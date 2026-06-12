#pragma once

#include <string>
#include <vector>

#include "database/Database.h"
#include "repositories/IUserRepository.h"
#include "models/User.h"

class UserRepository : public IUserRepository
{
   private:
    database::Database& database_;

   public:
    explicit UserRepository(database::Database& database);

    bool createUser(const User& user) override;
    User getUserById(int id) override;
    User getUserByUsername(const std::string& username) override;
    User getUserByEmail(const std::string& email) override;
    std::vector<User> getAllUsers() override;
    bool updateUser(const User& user) override;
    bool deleteUser(int id) override;
    bool updatePassword(int id, const std::string& passwordHash) override;
    bool setUserStatus(int id, const std::string& status) override;
    bool setForcePasswordChange(int id, bool forcePasswordChange) override;
    bool assignManager(int userId, int managerId) override;
};
