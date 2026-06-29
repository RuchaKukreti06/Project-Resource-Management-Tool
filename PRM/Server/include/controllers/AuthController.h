#pragma once

#include "services/interfaces/IAuthService.h"
#include "services/interfaces/ITokenService.h"
#include "repositories/IUserRepository.h"
#include "httplib.h"

class AuthController
{
   public:
    explicit AuthController(IAuthService& authService, ITokenService& tokenService, IUserRepository& userRepository);
    void registerRoutes(httplib::Server& server);
    void handleLogin(const httplib::Request& req, httplib::Response& res);
    void handleRegister(const httplib::Request& req, httplib::Response& res);
    void handleChangePassword(const httplib::Request& req, httplib::Response& res);

   private:
    IAuthService& authService_;
    ITokenService& tokenService_;
    IUserRepository& userRepository_;
};
