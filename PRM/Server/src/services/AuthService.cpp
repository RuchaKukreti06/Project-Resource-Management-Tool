#include "services/AuthService.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

AuthService::AuthService(std::shared_ptr<IUserRepository> repository,
                         std::shared_ptr<IPasswordHasher> passwordHasher,
                         std::shared_ptr<ITokenService> tokenService)
    : repository_(std::move(repository)), passwordHasher_(std::move(passwordHasher)), tokenService_(std::move(tokenService))
{
}

AuthService::~AuthService()
{
}

LoginResponse AuthService::login(const LoginRequest& req)
{
    LoginResponse response;
    response.success = false;

    User user = repository_->getUserByUsername(req.username);

    if (user.username.empty() || !passwordHasher_->verifyPassword(req.password, user.passwordHash))
    {
        response.message = "Invalid username or password.";
        return response;
    }

    if (user.status != "ACTIVE")
    {
        response.message = "Account is not active.";
        return response;
    }

    const std::string token = tokenService_->generateToken(user);
    response.success = true;
    response.token   = token;
    response.userId  = user.id;
    response.username = user.username;
    response.role = user.role;
    response.forcePasswordChange = user.forcePasswordChange;
    response.message = "Login successful.";
    return response;
}

RegisterResponse AuthService::registerUser(const RegisterRequest& req)
{
    RegisterResponse response;
    response.success = false;

    std::string validationMessage;
    if (!userValidator_.validateUsername(req.username, validationMessage))
    {
        response.message = validationMessage;
        return response;
    }

    if (repository_->getUserByUsername(req.username).username.empty())
    {
        User newUser;
        newUser.username            = req.username;
        newUser.passwordHash        = passwordHasher_->hashPassword(req.password);
        newUser.role                = "EMPLOYEE";
        newUser.email               = req.email;
        newUser.fullName            = req.fullName;
        newUser.isActive            = true;
        newUser.status              = "ACTIVE";
        newUser.forcePasswordChange = true;

        if (repository_->createUser(newUser))
        {
            response.success = true;
            response.message = "Registration successful.";
        }
        else
        {
            response.message = "Failed to create user. Email may already be in use.";
        }
    }
    else
    {
        response.message = "Username already exists.";
    }
    return response;
}

bool AuthService::isLoggedIn() const
{
    return token_.has_value() && tokenService_->validateToken(*token_);
}

void AuthService::setToken(const std::string& token)
{
    token_ = token;
}

bool AuthService::changePassword(const ResetPasswordRequest& req)
{
    const std::string newHash = passwordHasher_->hashPassword(req.newPassword);
    if (!repository_->updatePassword(req.userId, newHash))
    {
        return false;
    }
    return repository_->setForcePasswordChange(req.userId, false);
}

bool AuthService::validateToken(const std::string& token) const
{
    return tokenService_->validateToken(token);
}
