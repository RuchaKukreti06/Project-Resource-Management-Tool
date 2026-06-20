#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct UserResponse
{
    int id;
    std::string username;
    std::string fullName;
    std::string email;
    std::string role;
    std::string department;
    std::string designation;
    int managerId;
    std::string status;
    bool forcePasswordChange;
    std::string createdAt;
    std::string updatedAt;
};

inline void to_json(nlohmann::json& j, const UserResponse& dto)
{
    j = nlohmann::json{
        {"id", dto.id},
        {"username", dto.username},
        {"full_name", dto.fullName},
        {"email", dto.email},
        {"role", dto.role},
        {"department", dto.department},
        {"designation", dto.designation},
        {"manager_id", dto.managerId},
        {"status", dto.status},
        {"force_password_change", dto.forcePasswordChange},
        {"created_at", dto.createdAt},
        {"updated_at", dto.updatedAt}
    };
}
