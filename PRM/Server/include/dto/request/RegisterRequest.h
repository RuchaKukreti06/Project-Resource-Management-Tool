#pragma once
#include <string>

struct RegisterRequest
{
    std::string username;
    std::string password;
    std::string email;
    std::string fullName;
};
