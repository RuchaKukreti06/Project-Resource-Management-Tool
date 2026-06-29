#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct UserDTO
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

    static UserDTO fromJson(const nlohmann::json& j)
    {
        UserDTO u;
        u.id = j.value("id", 0);
        u.username = j.value("username", "");
        u.fullName = j.value("full_name", "");
        u.email = j.value("email", "");
        u.role = j.value("role", "");
        u.department = j.value("department", "");
        u.designation = j.value("designation", "");
        u.managerId = j.value("manager_id", 0);
        u.status = j.value("status", "");
        return u;
    }
};

struct RegisterUserRequest
{
    std::string username;
    std::string password;
    std::string email;
    std::string fullName;

    nlohmann::json toJson() const
    {
        return {
            {"username", username},
            {"password", password},
            {"email", email},
            {"full_name", fullName}
        };
    }
};

struct CreateUserRequest
{
    std::string username;
    std::string password;
    std::string role;
    std::string email;
    std::string fullName;
    std::string department;
    std::string designation;
    bool forcePasswordChange = true;

    nlohmann::json toJson() const
    {
        return {
            {"username", username},
            {"password", password},
            {"role", role},
            {"email", email},
            {"full_name", fullName},
            {"department", department},
            {"designation", designation},
            {"force_password_change", forcePasswordChange}
        };
    }
};

