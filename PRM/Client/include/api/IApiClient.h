#ifndef I_API_CLIENT_H
#define I_API_CLIENT_H
#include <nlohmann/json.hpp>
#include <string>

class IApiClient
{
   public:
    virtual ~IApiClient() = default;
    virtual nlohmann::json get(const std::string& endpoint) = 0;
    virtual nlohmann::json post(const std::string& endpoint, const nlohmann::json& payload) = 0;
    virtual nlohmann::json put(const std::string& endpoint, const nlohmann::json& payload) = 0;
    virtual nlohmann::json patch(const std::string& endpoint, const nlohmann::json& payload) = 0;
    virtual nlohmann::json del(const std::string& endpoint) = 0;
    
    virtual void setToken(const std::string& token) = 0;
    virtual void clearToken() = 0;
};

#endif
