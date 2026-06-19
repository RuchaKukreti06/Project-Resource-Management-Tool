#include "services/JwtTokenService.h"

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <spdlog/spdlog.h>

#include <chrono>

JwtTokenService::JwtTokenService(const AuthConfig& config) : config_(config)
{
}

bool JwtTokenService::validateToken(const std::string& token) const
{
    try
    {
        auto decoded  = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{config_.jwtSecret})
                            .with_issuer("PRM_Server");
        verifier.verify(decoded);
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Token validation failed: {}", e.what());
        return false;
    }
}

std::string JwtTokenService::generateToken(const User& user) const
{
    auto token = jwt::create()
                     .set_issuer("PRM_Server")
                     .set_type("JWT")
                     .set_issued_at(std::chrono::system_clock::now())
                     .set_expires_at(std::chrono::system_clock::now() +
                                     std::chrono::minutes(config_.jwtExpirationMinutes))
                     .set_payload_claim("id",       jwt::claim(std::to_string(user.id)))
                     .set_payload_claim("username", jwt::claim(user.username))
                     .set_payload_claim("role",     jwt::claim(user.role))
                     .sign(jwt::algorithm::hs256{config_.jwtSecret});

    return token;
}
