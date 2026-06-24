#pragma once

#include <string>

#include "ISessionStore.h"

namespace api
{

class AuthSession : public ISessionStore
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
               int userId, bool forcePasswordChange) override;
    void logout() override;
    bool isLoggedIn() const override;
    const std::string& token() const override;
    const std::string& username() const override;
    const std::string& role() const override;
    int userId() const override;
    bool forcePasswordChange() const override;
};

} 