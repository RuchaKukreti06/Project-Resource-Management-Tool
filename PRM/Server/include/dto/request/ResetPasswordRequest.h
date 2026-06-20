#pragma once
#include <string>

struct ResetPasswordRequest
{
    int userId;
    std::string newPassword;
};
