#pragma once

#include "services/interfaces/ITokenService.h"
#include "services/AuthConfig.h"

class JwtTokenService : public ITokenService {
public:
    explicit JwtTokenService(const AuthConfig& config);

    std::string generateToken(const User& user) const override;
    bool validateToken(const std::string& token) const override;
    std::string getClaimRole(const std::string& token) const override;
    int getClaimUserId(const std::string& token) const override;
    bool getClaimForcePasswordChange(const std::string& token) const override;

private:
    AuthConfig config_;
};
