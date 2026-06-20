#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct EmployeeResponse
{
    int id;
    int userId;
    std::string fullName;
    std::string email;
    std::string department;
    std::string designation;
    std::string status;
    bool isActive;
    int totalUtilisation;
};

inline void to_json(nlohmann::json& j, const EmployeeResponse& dto)
{
    j = nlohmann::json{
        {"id", dto.id},
        {"user_id", dto.userId},
        {"full_name", dto.fullName},
        {"email", dto.email},
        {"department", dto.department},
        {"designation", dto.designation},
        {"status", dto.status},
        {"is_active", dto.isActive},
        {"total_utilisation", dto.totalUtilisation}
    };
}
