#pragma once

#include "httplib.h"
#include "services/interfaces/ITokenService.h"

class AuthMiddleware
{
   public:
    explicit AuthMiddleware(ITokenService& tokenService);

    void registerMiddleware(httplib::Server& server);

   private:
    ITokenService& tokenService_;
};
