#include "ApiClient.h"
#include "ApiException.h"

#include <httplib.h>
#include "utils/Constants.h"

ApiClient::ApiClient(const std::string& baseUrl)
    : baseUrl_(baseUrl),
      client_(std::make_unique<httplib::Client>(baseUrl_))
{
    client_->set_connection_timeout(constants::CONNECTION_TIMEOUT_SEC, 0);
    client_->set_read_timeout(constants::READ_TIMEOUT_SEC, 0);
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

    if (response->status >= constants::HTTP_STATUS_OK_MIN && response->status <= constants::HTTP_STATUS_OK_MAX)
    {
        return parseSuccessResponse(response);
    }

    std::string errorMessage = parseErrorMessage(response);
    throwExceptionForStatus(response->status, errorMessage);
}

nlohmann::json ApiClient::parseSuccessResponse(const httplib::Result& response)
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

std::string ApiClient::parseErrorMessage(const httplib::Result& response)
{
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
        if (response->status == constants::HTTP_STATUS_NOT_FOUND) errorMessage = "Resource not found.";
        else if (response->status == constants::HTTP_STATUS_INTERNAL_SERVER_ERROR) errorMessage = "Internal server error.";
    }
    return errorMessage;
}

void ApiClient::throwExceptionForStatus(int status, const std::string& errorMessage)
{
    switch (status)
    {
        case constants::HTTP_STATUS_BAD_REQUEST:
            throw ValidationException(errorMessage);
        case constants::HTTP_STATUS_UNAUTHORIZED:
            throw AuthenticationException(errorMessage);
        case constants::HTTP_STATUS_FORBIDDEN:
            throw AuthorizationException(errorMessage);
        case constants::HTTP_STATUS_NOT_FOUND:
            throw NotFoundException(errorMessage);
        default:
            if (status >= constants::HTTP_STATUS_INTERNAL_SERVER_ERROR)
            {
                throw ServerException(errorMessage, status);
            }
            throw ApiException(errorMessage, status);
    }
}
