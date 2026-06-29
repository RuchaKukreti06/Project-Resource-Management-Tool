#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "models/User.h"
#include "repositories/IUserRepository.h"
#include "services/interfaces/IAuthService.h"
#include "services/interfaces/IPasswordHasher.h"
#include "services/interfaces/ITokenService.h"
#include "validators/UserValidator.h"

class AuthService : public IAuthService
{
   public:
    explicit AuthService(std::shared_ptr<IUserRepository> repository,
                         std::shared_ptr<IPasswordHasher> passwordHasher,
                         std::shared_ptr<ITokenService> tokenService);
    virtual ~AuthService();

    LoginResponse login(const LoginRequest& req) override;
    RegisterResponse registerUser(const RegisterRequest& req) override;
    bool changePassword(const ResetPasswordRequest& req) override;
    bool isLoggedIn() const override;
    void setToken(const std::string& token) override;
    bool validateToken(const std::string& token) const override;

   private:
    std::shared_ptr<IUserRepository> repository_;
    std::shared_ptr<IPasswordHasher> passwordHasher_;
    std::shared_ptr<ITokenService>   tokenService_;
    std::optional<std::string>       token_;
    UserValidator                    userValidator_;
};