#ifndef MOCK_API_CLIENT_H
#define MOCK_API_CLIENT_H

#include "IApiClient.h"
#include <map>
#include <string>
#include <functional>
#include <stdexcept>

class MockApiClient : public IApiClient
{
public:
    std::map<std::string, nlohmann::json> getResponses;
    std::map<std::string, nlohmann::json> postResponses;
    std::map<std::string, nlohmann::json> putResponses;
    std::map<std::string, nlohmann::json> patchResponses;
    std::map<std::string, nlohmann::json> deleteResponses;

    std::map<std::string, std::function<void()>> getExceptions;
    std::map<std::string, std::function<void()>> postExceptions;
    std::map<std::string, std::function<void()>> putExceptions;
    std::map<std::string, std::function<void()>> patchExceptions;
    std::map<std::string, std::function<void()>> deleteExceptions;

    std::string currentToken;

    nlohmann::json get(const std::string& endpoint) override
    {
        if (getExceptions.count(endpoint)) getExceptions[endpoint]();
        if (getResponses.count(endpoint)) return getResponses[endpoint];
        return nlohmann::json::object();
    }

    nlohmann::json post(const std::string& endpoint, const nlohmann::json& payload) override
    {
        if (postExceptions.count(endpoint)) postExceptions[endpoint]();
        if (postResponses.count(endpoint)) return postResponses[endpoint];
        return nlohmann::json::object();
    }

    nlohmann::json put(const std::string& endpoint, const nlohmann::json& payload) override
    {
        if (putExceptions.count(endpoint)) putExceptions[endpoint]();
        if (putResponses.count(endpoint)) return putResponses[endpoint];
        return nlohmann::json::object();
    }

    nlohmann::json patch(const std::string& endpoint, const nlohmann::json& payload) override
    {
        if (patchExceptions.count(endpoint)) patchExceptions[endpoint]();
        if (patchResponses.count(endpoint)) return patchResponses[endpoint];
        return nlohmann::json::object();
    }

    nlohmann::json del(const std::string& endpoint) override
    {
        if (deleteExceptions.count(endpoint)) deleteExceptions[endpoint]();
        if (deleteResponses.count(endpoint)) return deleteResponses[endpoint];
        return nlohmann::json::object();
    }

    void setToken(const std::string& token) override
    {
        currentToken = token;
    }

    void clearToken() override
    {
        currentToken.clear();
    }
};

#endif
