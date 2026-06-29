#pragma once
#include <string>

struct EmployeeCreateRequest
{
    int userId;
    std::string fullName;
    std::string email;
    std::string department;
    std::string designation;
};
