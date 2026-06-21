#pragma once

#include "httplib.h"
#include "repositories/ISystemConfigRepository.h"
#include <memory>

class SystemConfigController
{
   public:
    explicit SystemConfigController(std::shared_ptr<ISystemConfigRepository> configRepo);

    void registerRoutes(httplib::Server& server);

   private:
    void handleGetConfig(const httplib::Request& req, httplib::Response& res);
    void handleUpdateConfig(const httplib::Request& req, httplib::Response& res);

    std::shared_ptr<ISystemConfigRepository> configRepo_;
};
