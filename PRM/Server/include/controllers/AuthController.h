#pragma once

#include "AuthService.h"
#include "httplib.h"

class AuthController
{
   public:
    explicit AuthController(AuthService& authService);
    void registerRoutes(httplib::Server& server);
    void handleLogin(const httplib::Request& req, httplib::Response& res);
    void handleRegister(const httplib::Request& req, httplib::Response& res);
    void handleChangePassword(const httplib::Request& req, httplib::Response& res);

   private:
    AuthService& authService_;
};
