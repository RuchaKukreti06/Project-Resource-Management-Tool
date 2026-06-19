#pragma once

#include <string>
#include <nlohmann/json.hpp>

class IAuthService {
public:
    virtual ~IAuthService() = default;
    virtual nlohmann::json login(const std::string& username, const std::string& password) = 0;
    virtual nlohmann::json registerUser(const std::string& username, const std::string& password,
                                        const std::string& email, const std::string& fullName) = 0;
    virtual bool isLoggedIn() const = 0;
    virtual void setToken(const std::string& token) = 0;
    virtual bool changePassword(int userId, const std::string& newPassword) = 0;
    virtual bool validateToken(const std::string& token) const = 0;
};
