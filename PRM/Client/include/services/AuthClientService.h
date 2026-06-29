#ifndef AUTH_CLIENT_SERVICE_H
#define AUTH_CLIENT_SERVICE_H

#include "api/IApiClient.h"
#include "dto/AuthDTO.h"
#include "dto/ApiResponse.h"
#include <string>

class AuthClientService
{
private:
    IApiClient& apiClient_;

public:
    explicit AuthClientService(IApiClient& apiClient);

    AuthLoginResponse login(const std::string& username, const std::string& password);
    AuthChangePasswordResponse changePassword(int userId, const std::string& newPassword);
};

#endif // AUTH_CLIENT_SERVICE_H
