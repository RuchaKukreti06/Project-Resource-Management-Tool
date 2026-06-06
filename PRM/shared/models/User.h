#pragma once

#include <string>

struct User
{
    int id = 0;
    std::string username;
    std::string passwordHash;
    std::string role;
    std::string status;
    bool forcePasswordChange = false;
    std::string createdAt;
    std::string updatedAt;
};