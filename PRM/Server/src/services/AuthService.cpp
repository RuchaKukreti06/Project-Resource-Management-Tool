#include "services/AuthService.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "exceptions/Exceptions.h"

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
    User user = repository_->getUserByUsername(req.username);

    if (user.username.empty() || !passwordHasher_->verifyPassword(req.password, user.passwordHash))
    {
        throw exceptions::AuthenticationException("Invalid username or password.");
    }

    if (user.status != "ACTIVE")
    {
        throw exceptions::AuthenticationException("Account is not active.");
    }

    const std::string token = tokenService_->generateToken(user);
    
    LoginResponse response;
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
    std::string validationMessage;
    if (!userValidator_.validateUsername(req.username, validationMessage))
    {
        throw exceptions::ValidationException(validationMessage);
    }

    if (!repository_->getUserByUsername(req.username).username.empty())
    {
        throw exceptions::ConflictException("Username already exists.");
    }

    User newUser;
    newUser.username            = req.username;
    newUser.passwordHash        = passwordHasher_->hashPassword(req.password);
    newUser.role                = "EMPLOYEE";
    newUser.email               = req.email;
    newUser.fullName            = req.fullName;
    newUser.isActive            = true;
    newUser.status              = "ACTIVE";
    newUser.forcePasswordChange = true;

    if (!repository_->createUser(newUser))
    {
        throw exceptions::DatabaseException("Failed to create user. Email may already be in use.");
    }
    
    RegisterResponse response;
    response.success = true;
    response.message = "Registration successful.";
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
        throw exceptions::NotFoundException("Failed to change password. User not found.");
    }
    
    if (!repository_->setForcePasswordChange(req.userId, false))
    {
        throw exceptions::DatabaseException("Failed to update password force change status.");
    }
    return true;
}

bool AuthService::validateToken(const std::string& token) const
{
    return tokenService_->validateToken(token);
}
