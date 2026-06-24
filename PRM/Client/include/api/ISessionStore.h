#pragma once

#include <string>

namespace api
{

class ISessionStore
{
public:
    virtual ~ISessionStore() = default;

    virtual void login(const std::string& token, const std::string& username, const std::string& role,
                       int userId, bool forcePasswordChange) = 0;
    virtual void logout() = 0;
    
    virtual bool isLoggedIn() const = 0;
    virtual const std::string& token() const = 0;
    virtual const std::string& username() const = 0;
    virtual const std::string& role() const = 0;
    virtual int userId() const = 0;
    virtual bool forcePasswordChange() const = 0;
};

} // namespace api
