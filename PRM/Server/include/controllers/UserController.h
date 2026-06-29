#pragma once

#include "httplib.h"

#include "services/interfaces/IUserService.h"

class UserController
{
   public:
    explicit UserController(IUserService& userService);
    void registerRoutes(httplib::Server& server) const;

   private:
    IUserService& userService_;
};
