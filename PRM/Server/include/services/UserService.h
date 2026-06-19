#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "models/User.h"
#include "repositories/IUserRepository.h"
#include "services/interfaces/IUserService.h"
#include "services/interfaces/IPasswordHasher.h"

class UserService : public IUserService
{
   public:
    explicit UserService(std::shared_ptr<IUserRepository> repository,
                         std::shared_ptr<IPasswordHasher> passwordHasher);

    std::vector<User> getAllUsers() override;
    std::optional<User> getUserById(int id) override;
    User getUserByUsername(const std::string& username) override;
    bool createUser(const std::string& username, const std::string& password,
                    const std::string& role, const std::string& email,
                    const std::string& fullName, const std::string& department = "",
                    const std::string& designation = "",
                    bool forcePasswordChange = true) override;
    bool deactivateUser(int id) override;
    bool reactivateUser(int id) override;
    bool resetPassword(int id, const std::string& newPassword,
                       bool forcePasswordChange = true) override;
    bool assignManager(int userId, int managerId) override;

   private:
    std::shared_ptr<IUserRepository> repository_;
    std::shared_ptr<IPasswordHasher> passwordHasher_;
};
