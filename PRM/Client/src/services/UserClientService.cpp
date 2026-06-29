#include "services/UserClientService.h"

UserClientService::UserClientService(IApiClient& apiClient)
    : apiClient_(apiClient)
{
}

ApiEmptyResponse UserClientService::registerUser(const RegisterUserRequest& request)
{
    auto response = apiClient_.post("/auth/register", request.toJson());
    return ApiEmptyResponse::fromJson(response);
}

ApiEmptyResponse UserClientService::createUser(const CreateUserRequest& request)
{
    auto response = apiClient_.post("/users", request.toJson());
    return ApiEmptyResponse::fromJson(response);
}

ApiListResponse<UserDTO> UserClientService::viewUsers()
{
    auto response = apiClient_.get("/users");
    return ApiListResponse<UserDTO>::fromJson(response);
}

ApiEmptyResponse UserClientService::resetPassword(const std::string& userId, const std::string& newPassword, bool forcePasswordChange)
{
    auto response = apiClient_.put("/users/" + userId + "/reset-password", {
        {"new_password", newPassword},
        {"force_password_change", forcePasswordChange}
    });
    return ApiEmptyResponse::fromJson(response);
}

ApiEmptyResponse UserClientService::deactivateUser(const std::string& userId)
{
    auto response = apiClient_.put("/users/" + userId + "/deactivate", nlohmann::json::object());
    return ApiEmptyResponse::fromJson(response);
}

ApiEmptyResponse UserClientService::reactivateUser(const std::string& userId)
{
    auto response = apiClient_.put("/users/" + userId + "/reactivate", nlohmann::json::object());
    return ApiEmptyResponse::fromJson(response);
}
