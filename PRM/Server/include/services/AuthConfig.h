#pragma once

#include <string>

/// Value object carrying JWT configuration.
/// Injected into AuthService at construction — no ConfigLoader Singleton access at runtime.
struct AuthConfig
{
    std::string jwtSecret;
    int         jwtExpirationMinutes = 60;
};
