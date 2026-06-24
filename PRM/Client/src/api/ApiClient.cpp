#include "ApiClient.h"
#include "ApiException.h"

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
    return handleResponse(response);
}

nlohmann::json ApiClient::post(const std::string& endpoint, const nlohmann::json& payload)
{
    httplib::Headers headers;
    if (!authToken_.empty())
    {
        headers.emplace("Authorization", "Bearer " + authToken_);
    }
    auto response = client_->Post(endpoint, headers, payload.dump(), "application/json");
    return handleResponse(response);
}

nlohmann::json ApiClient::put(const std::string& endpoint, const nlohmann::json& payload)
{
    httplib::Headers headers;
    if (!authToken_.empty())
    {
        headers.emplace("Authorization", "Bearer " + authToken_);
    }
    auto response = client_->Put(endpoint, headers, payload.dump(), "application/json");
    return handleResponse(response);
}

nlohmann::json ApiClient::patch(const std::string& endpoint, const nlohmann::json& payload)
{
    httplib::Headers headers;
    if (!authToken_.empty())
    {
        headers.emplace("Authorization", "Bearer " + authToken_);
    }
    auto response = client_->Patch(endpoint, headers, payload.dump(), "application/json");
    return handleResponse(response);
}

nlohmann::json ApiClient::del(const std::string& endpoint)
{
    httplib::Headers headers;
    if (!authToken_.empty())
    {
        headers.emplace("Authorization", "Bearer " + authToken_);
    }
    auto response = client_->Delete(endpoint, headers);
    return handleResponse(response);
}

void ApiClient::setToken(const std::string& token)
{
    authToken_ = token;
}

void ApiClient::clearToken()
{
    authToken_.clear();
}

nlohmann::json ApiClient::handleResponse(const httplib::Result& response)
{
    if (!response)
    {
        throw NetworkException("Could not connect to server. Please check if the server is running.");
    }

    if (response->status >= 200 && response->status < 300)
    {
        if (response->body.empty()) {
            return nlohmann::json::object();
        }
        try {
            return nlohmann::json::parse(response->body);
        } catch (...) {
            throw ServerException("Invalid JSON response from server.", response->status);
        }
    }

    std::string errorMessage = "An unknown error occurred.";
    try
    {
        auto errorJson = nlohmann::json::parse(response->body);
        if (errorJson.contains("error") && errorJson["error"].is_string())
        {
            errorMessage = errorJson["error"].get<std::string>();
        }
        else if (errorJson.contains("message") && errorJson["message"].is_string())
        {
            errorMessage = errorJson["message"].get<std::string>();
        }
    }
    catch (...)
    {
        // If it's not JSON, maybe use a default message or raw body if we have to, but instructions say no raw body.
        if (response->status == 404) errorMessage = "Resource not found.";
        else if (response->status == 500) errorMessage = "Internal server error.";
    }

    switch (response->status)
    {
        case 400:
            throw ValidationException(errorMessage);
        case 401:
            throw AuthenticationException(errorMessage);
        case 403:
            throw AuthorizationException("You are not allowed to perform this action.");
        case 404:
            throw NotFoundException(errorMessage);
        default:
            if (response->status >= 500)
            {
                throw ServerException(errorMessage, response->status);
            }
            throw ApiException(errorMessage, response->status);
    }
}
