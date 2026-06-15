#pragma once

#include "httplib.h"

#include "services/UserService.h"

class UserController
{
   public:
    explicit UserController(UserService& userService);
    void registerRoutes(httplib::Server& server) const;

   private:
    UserService& userService_;
};
