#ifndef USER_CLIENT_SERVICE_H
#define USER_CLIENT_SERVICE_H

#include "api/IApiClient.h"
#include "dto/UserDTO.h"
#include "dto/ApiResponse.h"
#include <string>

class UserClientService
{
private:
    IApiClient& apiClient_;

public:
    explicit UserClientService(IApiClient& apiClient);

    ApiEmptyResponse registerUser(const RegisterUserRequest& request);
    ApiEmptyResponse createUser(const CreateUserRequest& request);
    ApiListResponse<UserDTO> viewUsers();
    ApiEmptyResponse resetPassword(const std::string& userId, const std::string& newPassword, bool forcePasswordChange);
    ApiEmptyResponse deactivateUser(const std::string& userId);
    ApiEmptyResponse reactivateUser(const std::string& userId);
};

#endif // USER_CLIENT_SERVICE_H
