#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "models/User.h"
#include "repositories/IUserRepository.h"

class UserService
{
   public:
    explicit UserService(std::shared_ptr<IUserRepository> repository);

    std::vector<User> getAllUsers();
    std::optional<User> getUserById(int id);
    User getUserByUsername(const std::string& username);
    bool createUser(const std::string& username, const std::string& password,
                    const std::string& role, const std::string& email,
                    const std::string& fullName, const std::string& department = "",
                    const std::string& designation = "",
                    bool forcePasswordChange = true);
    bool deactivateUser(int id);
    bool reactivateUser(int id);
    bool resetPassword(int id, const std::string& newPassword,
                       bool forcePasswordChange = true);
    bool assignManager(int userId, int managerId);

   private:
    std::string hashPassword(const std::string& password) const;

    std::shared_ptr<IUserRepository> repository_;
};
