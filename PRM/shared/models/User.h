#pragma once

#include <string>

struct User
{
    int id = 0;
    std::string username;
    std::string passwordHash;
    // role is populated as a string from a JOIN with the roles table
    std::string role;
    int roleId = 0;
    std::string fullName;
    std::string email;
    std::string department;
    std::string designation;
    // status is a convenience string derived from is_active ("ACTIVE" / "INACTIVE")
    std::string status;
    bool isActive = true;
    bool forcePasswordChange = true;
    int managerId = 0;
    std::string createdAt;
    std::string updatedAt;
};