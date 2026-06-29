#pragma once

#include "services/interfaces/IPasswordHasher.h"

class PasswordHasher : public IPasswordHasher {
public:
    std::string hashPassword(const std::string& password) const override;
    bool verifyPassword(const std::string& password, const std::string& hash) const override;
};
