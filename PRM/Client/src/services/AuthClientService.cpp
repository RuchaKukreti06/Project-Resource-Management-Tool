#include "services/AuthClientService.h"

AuthClientService::AuthClientService(IApiClient& apiClient)
    : apiClient_(apiClient)
{
}

AuthLoginResponse AuthClientService::login(const std::string& username, const std::string& password)
{
    auto response = apiClient_.post("/auth/login", {
        {"username", username},
        {"password", password}
    });
    return AuthLoginResponse::fromJson(response);
}

AuthChangePasswordResponse AuthClientService::changePassword(int userId, const std::string& newPassword)
{
    auto response = apiClient_.post("/auth/change-password", {
        {"user_id", userId},
        {"new_password", newPassword}
    });
    return AuthChangePasswordResponse::fromJson(response);
}
