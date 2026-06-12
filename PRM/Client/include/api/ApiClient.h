#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <httplib.h>

#include <nlohmann/json.hpp>
#include <string>

#include "IClient.h"

class ApiClient : public IClient
{
   private:
    std::string baseUrl_;
    std::string authToken_;
    std::unique_ptr<httplib::Client> client_;

   public:
    ApiClient(const std::string& baseUrl);
    nlohmann::json get(const std::string& endpoint) override;
    nlohmann::json post(const std::string& endpoint, const nlohmann::json& payload) override;
    nlohmann::json put(const std::string& endpoint, const nlohmann::json& payload) override;
    nlohmann::json patch(const std::string& endpoint, const nlohmann::json& payload) override;
    nlohmann::json del(const std::string& endpoint) override;

    void setToken(const std::string& token);
    void clearToken();
};

#endif
