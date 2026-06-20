#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct LoginResponse
{
    bool success;
    std::string message;
    std::string token;
    
    int userId;
    std::string username;
    std::string role;
    bool forcePasswordChange;
};

inline void to_json(nlohmann::json& j, const LoginResponse& dto)
{
    j = nlohmann::json{
        {"success", dto.success},
        {"message", dto.message},
        {"token", dto.token},
        {"user", {
            {"id", dto.userId},
            {"username", dto.username},
            {"role", dto.role},
            {"force_password_change", dto.forcePasswordChange}
        }}
    };
}
