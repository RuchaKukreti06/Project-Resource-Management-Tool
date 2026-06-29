#pragma once

#include <string>
#include "models/User.h"

class ITokenService {
public:
    virtual ~ITokenService() = default;
    virtual std::string generateToken(const User& user) const = 0;
    virtual bool validateToken(const std::string& token) const = 0;
    virtual std::string getClaimRole(const std::string& token) const = 0;
    virtual int getClaimUserId(const std::string& token) const = 0;
    virtual bool getClaimForcePasswordChange(const std::string& token) const = 0;
};
