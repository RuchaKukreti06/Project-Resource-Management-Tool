#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <httplib.h>

#include <nlohmann/json.hpp>
#include <string>

#include "IApiClient.h"

class ApiClient : public IApiClient
{
   private:
    std::string baseUrl_;
    std::string authToken_;
    std::unique_ptr<httplib::Client> client_;

    nlohmann::json handleResponse(const httplib::Result& response);

   public:
    ApiClient(const std::string& baseUrl);
    nlohmann::json get(const std::string& endpoint) override;
    nlohmann::json post(const std::string& endpoint, const nlohmann::json& payload) override;
    nlohmann::json put(const std::string& endpoint, const nlohmann::json& payload) override;
    nlohmann::json patch(const std::string& endpoint, const nlohmann::json& payload) override;
    nlohmann::json del(const std::string& endpoint) override;

    void setToken(const std::string& token) override;
    void clearToken() override;
};

#endif
