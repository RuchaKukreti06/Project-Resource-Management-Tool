#include "AuthSession.h"

namespace api
{

AuthSession& AuthSession::instance()
{
    static AuthSession session;
    return session;
}

void AuthSession::login(const std::string& token, const std::string& username,
                        const std::string& role, int userId, bool forcePasswordChange)
{
    token_ = token;
    username_ = username;
    role_ = role;
    userId_ = userId;
    forcePasswordChange_ = forcePasswordChange;
    loggedIn_ = true;
}

void AuthSession::logout()
{
    token_.clear();
    username_.clear();
    role_.clear();
    userId_ = 0;
    forcePasswordChange_ = false;
    loggedIn_ = false;
}

bool AuthSession::isLoggedIn() const
{
    return loggedIn_;
}

const std::string& AuthSession::token() const
{
    return token_;
}

const std::string& AuthSession::username() const
{
    return username_;
}

const std::string& AuthSession::role() const
{
    return role_;
}

int AuthSession::userId() const
{
    return userId_;
}

bool AuthSession::forcePasswordChange() const
{
    return forcePasswordChange_;
}

}  // namespace api
