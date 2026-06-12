#ifndef I_CLIENT_H
#define I_CLIENT_H
#include <nlohmann/json.hpp>

class IClient
{
   public:
    virtual nlohmann::json get(const std::string& endpoint) = 0;
    virtual nlohmann::json post(const std::string& endpoint, const nlohmann::json& payload) = 0;
    virtual nlohmann::json put(const std::string& endpoint, const nlohmann::json& payload) = 0;
    virtual nlohmann::json patch(const std::string& endpoint, const nlohmann::json& payload) = 0;
    virtual nlohmann::json del(const std::string& endpoint) = 0;
};

#endif
