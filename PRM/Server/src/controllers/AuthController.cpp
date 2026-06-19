#include "controllers/AuthController.h"

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
    try
    {
        auto payload = nlohmann::json::parse(req.body);

        auto username = payload.at("username").get<std::string>();

        auto password = payload.at("password").get<std::string>();

        auto response = authService_.login(username, password);

        res.status = 200;

        res.set_content(response.dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        nlohmann::json response;

        response["success"] = false;
        response["message"] = std::string("Invalid request body: ") + e.what();

        res.status = 400;

        res.set_content(response.dump(), "application/json");
    }
}

void AuthController::handleRegister(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        auto payload = nlohmann::json::parse(req.body);

        auto username = payload.at("username").get<std::string>();
        auto password = payload.at("password").get<std::string>();
        auto email    = payload.at("email").get<std::string>();
        auto fullName = payload.at("full_name").get<std::string>();

        auto response = authService_.registerUser(username, password, email, fullName);

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        nlohmann::json response;
        response["success"] = false;
        response["message"] = std::string("Invalid request body: ") + e.what();
        res.status = 400;
        res.set_content(response.dump(), "application/json");
    }
}

void AuthController::handleChangePassword(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        auto payload = nlohmann::json::parse(req.body);

        // D12: Accept snake_case keys; fall back to camelCase for compatibility
        int userId = 0;
        if (payload.contains("user_id"))
            userId = payload.at("user_id").get<int>();
        else
            userId = payload.at("userId").get<int>();

        std::string newPassword;
        if (payload.contains("new_password"))
            newPassword = payload.at("new_password").get<std::string>();
        else
            newPassword = payload.at("newPassword").get<std::string>();

        bool success = authService_.changePassword(userId, newPassword);

        nlohmann::json response;

        response["success"] = success;

        if (success)
        {
            response["message"] = "Password changed successfully.";

            res.status = 200;
        }
        else
        {
            response["message"] = "Failed to change password.";

            res.status = 500;
        }

        res.set_content(response.dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        nlohmann::json response;

        response["success"] = false;
        response["message"] = std::string("Invalid request body: ") + e.what();

        res.status = 400;

        res.set_content(response.dump(), "application/json");
    }
}