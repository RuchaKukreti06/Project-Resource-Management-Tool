#include "ApiClient.h"

#include <httplib.h>

ApiClient::ApiClient(const std::string& baseUrl)
    : baseUrl_(baseUrl),
      client_(std::make_unique<httplib::Client>("127.0.0.1", 8080))  // constructed once, correctly
{
    client_->set_connection_timeout(5, 0);
    client_->set_read_timeout(60, 0);  // 60 s — LLM calls may take up to 60 s

}

nlohmann::json ApiClient::get(const std::string& endpoint)
{
    httplib::Headers headers;
    if (!authToken_.empty())
    {
        headers.emplace("Authorization", "Bearer " + authToken_);
    }
    auto response = client_->Get(endpoint, headers);
    if (!response)
        throw std::runtime_error("GET " + endpoint + " — no response (server down?)");
    if (response->status < 200 || response->status >= 300)
        throw std::runtime_error("GET " + endpoint + " returned HTTP " +
                                 std::to_string(response->status) + ": " + response->body);
    return nlohmann::json::parse(response->body);
}

nlohmann::json ApiClient::post(const std::string& endpoint, const nlohmann::json& payload)
{
    httplib::Headers headers;
    if (!authToken_.empty())
    {
        headers.emplace("Authorization", "Bearer " + authToken_);
    }
    auto response = client_->Post(endpoint, headers, payload.dump(), "application/json");
    if (!response)
        throw std::runtime_error("POST " + endpoint + " — no response (server down?)");
    if (response->status < 200 || response->status >= 300)
        throw std::runtime_error("POST " + endpoint + " returned HTTP " +
                                 std::to_string(response->status) + ": " + response->body);
    return nlohmann::json::parse(response->body);
}

nlohmann::json ApiClient::put(const std::string& endpoint, const nlohmann::json& payload)
{
    httplib::Headers headers;
    if (!authToken_.empty())
    {
        headers.emplace("Authorization", "Bearer " + authToken_);
    }
    auto response = client_->Put(endpoint, headers, payload.dump(), "application/json");
    if (!response)
        throw std::runtime_error("PUT " + endpoint + " — no response (server down?)");
    if (response->status < 200 || response->status >= 300)
        throw std::runtime_error("PUT " + endpoint + " returned HTTP " +
                                 std::to_string(response->status) + ": " + response->body);
    return nlohmann::json::parse(response->body);
}

nlohmann::json ApiClient::patch(const std::string& endpoint, const nlohmann::json& payload)
{
    httplib::Headers headers;
    if (!authToken_.empty())
    {
        headers.emplace("Authorization", "Bearer " + authToken_);
    }
    auto response = client_->Patch(endpoint, headers, payload.dump(), "application/json");
    if (!response)
        throw std::runtime_error("PATCH " + endpoint + " — no response (server down?)");
    if (response->status < 200 || response->status >= 300)
        throw std::runtime_error("PATCH " + endpoint + " returned HTTP " +
                                 std::to_string(response->status) + ": " + response->body);
    return nlohmann::json::parse(response->body);
}

nlohmann::json ApiClient::del(const std::string& endpoint)
{
    httplib::Headers headers;
    if (!authToken_.empty())
    {
        headers.emplace("Authorization", "Bearer " + authToken_);
    }
    auto response = client_->Delete(endpoint, headers);
    if (!response)
        throw std::runtime_error("DELETE " + endpoint + " — no response (server down?)");
    if (response->status < 200 || response->status >= 300)
        throw std::runtime_error("DELETE " + endpoint + " returned HTTP " +
                                 std::to_string(response->status) + ": " + response->body);
    return nlohmann::json::parse(response->body);
}

void ApiClient::setToken(const std::string& token)
{
    authToken_ = token;
}

void ApiClient::clearToken()
{
    authToken_.clear();
}
