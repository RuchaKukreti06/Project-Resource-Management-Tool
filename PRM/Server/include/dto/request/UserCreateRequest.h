#pragma once
#include <string>

struct UserCreateRequest
{
    std::string username;
    std::string password;
    std::string role;
    std::string email;
    std::string fullName;
    std::string department;
    std::string designation;
    bool forcePasswordChange;
    int managerId;
};
