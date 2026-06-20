#include "controllers/AuthController.h"

#include "dto/DTOMapper.h"
#include <nlohmann/json.hpp>

AuthController::AuthController(IAuthService& authService) : authService_(authService)
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

    // D12: Accept snake_case keys; fall back to camelCase for compatibility
    ResetPasswordRequest request;
    if (payload.contains("user_id"))
        request.userId = payload.at("user_id").get<int>();
    else
        request.userId = payload.at("userId").get<int>();

    if (payload.contains("new_password"))
        request.newPassword = payload.at("new_password").get<std::string>();
    else
        request.newPassword = payload.at("newPassword").get<std::string>();

    bool success = authService_.changePassword(request);

    nlohmann::json response;
    response["success"] = success;
    response["message"] = "Password changed successfully.";

    res.status = 200;
    res.set_content(response.dump(), "application/json");
}