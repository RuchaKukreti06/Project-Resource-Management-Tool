#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct RegisterResponse
{
    bool success;
    std::string message;
};

inline void to_json(nlohmann::json& j, const RegisterResponse& dto)
{
    j = nlohmann::json{
        {"success", dto.success},
        {"message", dto.message}
    };
}
