#pragma once

#include <memory>
#include "httplib.h"
#include "services/AIService.h"
#include "repositories/ISystemConfigRepository.h"

class AIController
{
   public:
    explicit AIController(std::shared_ptr<AIService> aiService, std::shared_ptr<ISystemConfigRepository> configRepo);
    void registerRoutes(httplib::Server& server);

   private:
    void handleSkillMatch(const httplib::Request& req, httplib::Response& res);
    void handleRiskSummary(const httplib::Request& req, httplib::Response& res);
    void handleTeamBuilder(const httplib::Request& req, httplib::Response& res);

    std::shared_ptr<AIService> aiService_;
    std::shared_ptr<ISystemConfigRepository> configRepo_;
};
