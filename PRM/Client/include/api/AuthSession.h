#pragma once

#include <string>

namespace api
{

class AuthSession
{
   private:
    AuthSession() = default;
    std::string token_;
    std::string username_;
    std::string role_;
    int userId_ = 0;
    bool forcePasswordChange_ = false;
    bool loggedIn_ = false;

   public:
    static AuthSession& instance();
    void login(const std::string& token, const std::string& username, const std::string& role,
               int userId, bool forcePasswordChange);
    void logout();
    bool isLoggedIn() const;
    const std::string& token() const;
    const std::string& username() const;
    const std::string& role() const;
    int userId() const;
    bool forcePasswordChange() const;
};

} 