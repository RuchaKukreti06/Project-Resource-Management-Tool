#pragma once

#include "services/interfaces/IAuthService.h"
#include "httplib.h"

class AuthController
{
   public:
    explicit AuthController(IAuthService& authService);
    void registerRoutes(httplib::Server& server);
    void handleLogin(const httplib::Request& req, httplib::Response& res);
    void handleRegister(const httplib::Request& req, httplib::Response& res);
    void handleChangePassword(const httplib::Request& req, httplib::Response& res);

   private:
    IAuthService& authService_;
};
