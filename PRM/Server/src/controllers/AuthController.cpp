#include "controllers/AuthController.h"

#include "dto/DTOMapper.h"
#include <nlohmann/json.hpp>
#include "exceptions/Exceptions.h"

AuthController::AuthController(IAuthService& authService, ITokenService& tokenService, IUserRepository& userRepository)
    : authService_(authService), tokenService_(tokenService), userRepository_(userRepository)
{
}

void AuthController::registerRoutes(httplib::Server& server)
{
    server.Post("/auth/login", [this](const httplib::Request& req, httplib::Response& res)
                { handleLogin(req, res); });

    server.Post("/auth/register", [this](const httplib::Request& req, httplib::Response& res)
                { handleRegister(req, res); });

    server.Post("/auth/change-password", [this](const httplib::Request& req, httplib::Response& res)
                { handleChangePassword(req, res); });
}

void AuthController::handleLogin(const httplib::Request& req, httplib::Response& res)
{
    auto payload = nlohmann::json::parse(req.body);

    LoginRequest loginReq;
    loginReq.username = payload.at("username").get<std::string>();
    loginReq.password = payload.at("password").get<std::string>();

    auto response = authService_.login(loginReq);

    res.status = 200;
    res.set_content(nlohmann::json(response).dump(), "application/json");
}

void AuthController::handleRegister(const httplib::Request& req, httplib::Response& res)
{
    auto payload = nlohmann::json::parse(req.body);

    RegisterRequest registerReq;
    registerReq.username = payload.at("username").get<std::string>();
    registerReq.password = payload.at("password").get<std::string>();
    registerReq.email    = payload.at("email").get<std::string>();
    registerReq.fullName = payload.at("full_name").get<std::string>();

    auto response = authService_.registerUser(registerReq);

    res.status = 200;
    res.set_content(nlohmann::json(response).dump(), "application/json");
}

void AuthController::handleChangePassword(const httplib::Request& req, httplib::Response& res)
{
    auto payload = nlohmann::json::parse(req.body);

    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.find("Bearer ") != 0) {
        throw exceptions::AuthenticationException("Missing or invalid authorization header.");
    }
    std::string token = authHeader.substr(7);

    ResetPasswordRequest request;
    request.userId = tokenService_.getClaimUserId(token);

    if (payload.contains("new_password"))
        request.newPassword = payload.at("new_password").get<std::string>();
    else
        request.newPassword = payload.at("newPassword").get<std::string>();

    bool success = authService_.changePassword(request);

    nlohmann::json response;
    response["success"] = success;
    if (success) {
        response["message"] = "Password changed successfully.";
        
        User user = userRepository_.getUserById(request.userId);
        if (user.id > 0) {
            std::string newToken = tokenService_.generateToken(user);
            response["token"] = newToken;
        }
    } else {
        response["message"] = "Failed to change password.";
    }

    res.status = 200;
    res.set_content(response.dump(), "application/json");
}