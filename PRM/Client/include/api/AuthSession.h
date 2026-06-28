#pragma once

#include <string>

#include "ISessionStore.h"

namespace api
{

class AuthSession : public ISessionStore
{
   public:
    AuthSession() = default;
   private:
    SessionData data_;
    bool loggedIn_ = false;

   public:
    void login(const SessionData& data) override;
    void logout() override;
    bool isLoggedIn() const override;
    const std::string& token() const override;
    const std::string& username() const override;
    const std::string& role() const override;
    int userId() const override;
    bool forcePasswordChange() const override;
};

} 