#pragma once

#include <string>
#include <optional>
#include <vector>
#include <nlohmann/json.hpp>

template <typename T>
struct ApiResponse
{
    bool success;
    std::string message;
    std::optional<T> data;

    static ApiResponse<T> fromJson(const nlohmann::json& j)
    {
        ApiResponse<T> response;
        response.success = j.value("success", false);
        response.message = j.value("message", "");
        
        if (response.success && j.contains("data") && !j["data"].is_null())
        {
            response.data = T::fromJson(j["data"]);
        }
        return response;
    }
};

template <typename T>
struct ApiListResponse
{
    bool success;
    std::string message;
    std::vector<T> data;

    static ApiListResponse<T> fromJson(const nlohmann::json& j)
    {
        ApiListResponse<T> response;
        response.success = j.value("success", false);
        response.message = j.value("message", "");
        
        if (response.success && j.contains("data") && j["data"].is_array())
        {
            for (const auto& item : j["data"])
            {
                response.data.push_back(T::fromJson(item));
            }
        }
        return response;
    }
};

struct ApiEmptyResponse
{
    bool success;
    std::string message;

    static ApiEmptyResponse fromJson(const nlohmann::json& j)
    {
        ApiEmptyResponse response;
        response.success = j.value("success", false);
        response.message = j.value("message", "");
        return response;
    }
};
