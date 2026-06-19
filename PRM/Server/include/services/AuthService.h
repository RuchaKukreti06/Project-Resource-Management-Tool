#ifndef SERVER_AUTH_SERVICE_H
#define SERVER_AUTH_SERVICE_H

#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "models/User.h"
#include "repositories/IUserRepository.h"
#include "services/AuthConfig.h"
#include "services/interfaces/IAuthService.h"
#include "services/interfaces/IPasswordHasher.h"

class AuthService : public IAuthService
{
   public:
    explicit AuthService(std::shared_ptr<IUserRepository> repository,
                         std::shared_ptr<IPasswordHasher> passwordHasher,
                         const AuthConfig& config);
    ~AuthService();

    nlohmann::json login(const std::string& username, const std::string& password) override;
    nlohmann::json registerUser(const std::string& username, const std::string& password,
                                const std::string& email, const std::string& fullName) override;
    bool isLoggedIn() const override;
    void setToken(const std::string& token) override;
    bool changePassword(int userId, const std::string& newPassword) override;
    bool validateToken(const std::string& token) const override;

   private:
    std::string generateToken(const User& user);

    std::shared_ptr<IUserRepository> repository_;
    std::shared_ptr<IPasswordHasher> passwordHasher_;
    std::optional<std::string>       token_;
    AuthConfig                       config_;
};

#endif