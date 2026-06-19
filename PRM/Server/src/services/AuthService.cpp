#include "services/AuthService.h"

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

AuthService::AuthService(std::shared_ptr<IUserRepository> repository,
                         std::shared_ptr<IPasswordHasher> passwordHasher,
                         const AuthConfig& config)
    : repository_(std::move(repository)), passwordHasher_(std::move(passwordHasher)), config_(std::move(config))
{
}

AuthService::~AuthService()
{
}

nlohmann::json AuthService::login(const std::string& username, const std::string& password)
{
    nlohmann::json response;
    response["success"] = false;

    User user = repository_->getUserByUsername(username);

    if (user.username.empty() || !passwordHasher_->verifyPassword(password, user.passwordHash))
    {
        response["message"] = "Invalid username or password.";
        return response;
    }

    if (user.status != "ACTIVE")
    {
        response["message"] = "Account is not active.";
        return response;
    }

    std::string token = generateToken(user);
    response["success"] = true;
    response["token"]   = token;
    response["user"]    = {{"id",                    user.id},
                            {"username",              user.username},
                            {"role",                  user.role},
                            {"force_password_change", user.forcePasswordChange}};
    response["message"] = "Login successful.";
    return response;
}

nlohmann::json AuthService::registerUser(const std::string& username,
                                         const std::string& password,
                                         const std::string& email,
                                         const std::string& fullName)
{
    nlohmann::json response;

    if (repository_->getUserByUsername(username).username.empty())
    {
        User newUser;
        newUser.username            = username;
        newUser.passwordHash        = passwordHasher_->hashPassword(password);
        newUser.role                = "EMPLOYEE";
        newUser.email               = email;
        newUser.fullName            = fullName;
        newUser.isActive            = true;
        newUser.status              = "ACTIVE";
        newUser.forcePasswordChange = true;

        if (repository_->createUser(newUser))
        {
            response["success"] = true;
            response["message"] = "Registration successful.";
        }
        else
        {
            response["success"] = false;
            response["message"] = "Failed to create user. Email may already be in use.";
        }
    }
    else
    {
        response["success"] = false;
        response["message"] = "Username already exists.";
    }
    return response;
}

bool AuthService::isLoggedIn() const
{
    return token_.has_value() && validateToken(*token_);
}

void AuthService::setToken(const std::string& token)
{
    token_ = token;
}

bool AuthService::changePassword(int userId, const std::string& newPassword)
{
    const std::string newHash = passwordHasher_->hashPassword(newPassword);
    if (!repository_->updatePassword(userId, newHash))
    {
        return false;
    }
    return repository_->setForcePasswordChange(userId, false);
}

bool AuthService::validateToken(const std::string& token) const
{
    try
    {
        auto decoded  = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{config_.jwtSecret})
                            .with_issuer("PRM_Server");
        verifier.verify(decoded);
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Token validation failed: {}", e.what());
        return false;
    }
}

std::string AuthService::generateToken(const User& user)
{
    auto token = jwt::create()
                     .set_issuer("PRM_Server")
                     .set_type("JWT")
                     .set_issued_at(std::chrono::system_clock::now())
                     .set_expires_at(std::chrono::system_clock::now() +
                                     std::chrono::minutes(config_.jwtExpirationMinutes))
                     .set_payload_claim("id",       jwt::claim(std::to_string(user.id)))
                     .set_payload_claim("username", jwt::claim(user.username))
                     .set_payload_claim("role",     jwt::claim(user.role))
                     .sign(jwt::algorithm::hs256{config_.jwtSecret});

    return token;
}


