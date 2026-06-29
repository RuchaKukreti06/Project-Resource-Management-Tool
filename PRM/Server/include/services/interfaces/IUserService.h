#pragma once

#include <vector>
#include <optional>
#include <string>
#include "models/User.h"

#include "dto/request/UserCreateRequest.h"
#include "dto/response/UserResponse.h"

class IUserService {
public:
    virtual ~IUserService() = default;
    virtual std::vector<UserResponse> getAllUsers() = 0;
    virtual std::optional<UserResponse> getUserById(int id) = 0;
    virtual User getUserByUsername(const std::string& username) = 0;
    virtual bool createUser(const UserCreateRequest& req) = 0;
    virtual bool deactivateUser(int id) = 0;
    virtual bool reactivateUser(int id) = 0;
    virtual bool resetPassword(int id, const std::string& newPassword, bool forcePasswordChange = true) = 0;
    virtual bool assignManager(int userId, int managerId) = 0;
};
