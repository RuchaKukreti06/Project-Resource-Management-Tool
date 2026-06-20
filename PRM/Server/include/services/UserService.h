#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "models/User.h"
#include "repositories/IUserRepository.h"
#include "services/interfaces/IUserService.h"
#include "services/interfaces/IPasswordHasher.h"
#include "validators/UserValidator.h"

class UserService : public IUserService
{
   public:
    explicit UserService(std::shared_ptr<IUserRepository> repository,
                         std::shared_ptr<IPasswordHasher> passwordHasher);
    virtual ~UserService() = default;

    std::vector<UserResponse> getAllUsers() override;
    std::optional<UserResponse> getUserById(int userId) override;
    User getUserByUsername(const std::string& username) override;
    bool createUser(const UserCreateRequest& req) override;
    bool deactivateUser(int id) override;
    bool reactivateUser(int id) override;
    bool resetPassword(int id, const std::string& newPassword,
                       bool forcePasswordChange = true) override;
    bool assignManager(int userId, int managerId) override;

   private:
    std::shared_ptr<IUserRepository> repository_;
    std::shared_ptr<IPasswordHasher> passwordHasher_;
    UserValidator                    userValidator_;
};
