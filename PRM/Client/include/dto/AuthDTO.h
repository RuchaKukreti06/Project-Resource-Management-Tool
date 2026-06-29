#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct AuthLoginUserDTO
{
    int id;
    std::string username;
    std::string role;
    bool forcePasswordChange;

    static AuthLoginUserDTO fromJson(const nlohmann::json& j)
    {
        AuthLoginUserDTO u;
        u.id = j.value("id", 0);
        u.username = j.value("username", "");
        u.role = j.value("role", "");
        u.forcePasswordChange = j.value("force_password_change", false);
        return u;
    }
};

struct AuthLoginResponse
{
    bool success;
    std::string message;
    std::string token;
    AuthLoginUserDTO user;

    static AuthLoginResponse fromJson(const nlohmann::json& j)
    {
        AuthLoginResponse r;
        r.success = j.value("success", false);
        r.message = j.value("message", "");
        r.token = j.value("token", "");
        if (j.contains("user")) {
            r.user = AuthLoginUserDTO::fromJson(j["user"]);
        }
        return r;
    }
};

struct AuthChangePasswordResponse
{
    bool success;
    std::string message;
    std::string token;

    static AuthChangePasswordResponse fromJson(const nlohmann::json& j)
    {
        AuthChangePasswordResponse r;
        r.success = j.value("success", false);
        r.message = j.value("message", "");
        r.token = j.value("token", "");
        return r;
    }
};
