#pragma once

#include <string>
#include <vector>

#include "Database.h"
#include "IUserRepository.h"
#include "User.h"

class UserRepository : public IUserRepository
{
   private:
    database::Database& database_;

   public:
    explicit UserRepository(database::Database& database);

    bool createUser(const User& user) override;
    User getUserById(int id) override;
    User getUserByUsername(const std::string& username) override;
    std::vector<User> getAllUsers() override;
    bool updateUser(const User& user) override;
    bool deleteUser(int id) override;
    bool updatePassword(int id, const std::string& passwordHash) override;
};
