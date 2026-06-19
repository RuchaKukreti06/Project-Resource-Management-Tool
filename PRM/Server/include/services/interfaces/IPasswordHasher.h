#pragma once

#include <string>

class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;
    virtual std::string hashPassword(const std::string& password) const = 0;
    virtual bool verifyPassword(const std::string& password, const std::string& hash) const = 0;
};
