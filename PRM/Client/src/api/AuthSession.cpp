#include "AuthSession.h"

namespace api
{

void AuthSession::login(const SessionData& data)
{
    data_ = data;
    loggedIn_ = true;
}

void AuthSession::logout()
{
    data_ = SessionData{};
    loggedIn_ = false;
}

bool AuthSession::isLoggedIn() const
{
    return loggedIn_;
}

const std::string& AuthSession::token() const
{
    return data_.token;
}

const std::string& AuthSession::username() const
{
    return data_.username;
}

const std::string& AuthSession::role() const
{
    return data_.role;
}

int AuthSession::userId() const
{
    return data_.userId;
}

bool AuthSession::forcePasswordChange() const
{
    return data_.forcePasswordChange;
}

}  // namespace api
