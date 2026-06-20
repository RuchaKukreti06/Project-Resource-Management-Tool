#pragma once

#include <string>
#include "dto/request/LoginRequest.h"
#include "dto/request/RegisterRequest.h"
#include "dto/request/ResetPasswordRequest.h"
#include "dto/response/LoginResponse.h"
#include "dto/response/RegisterResponse.h"

class IAuthService
{
public:
    virtual ~IAuthService() = default;

    virtual LoginResponse login(const LoginRequest& req) = 0;
    virtual RegisterResponse registerUser(const RegisterRequest& req) = 0;
    virtual bool isLoggedIn() const = 0;
    virtual void setToken(const std::string& token) = 0;
    virtual bool changePassword(const ResetPasswordRequest& req) = 0;
    virtual bool validateToken(const std::string& token) const = 0;
};
