#pragma once

#include "httplib.h"

#include "services/SchedulerService.h"

class SchedulerController
{
   public:
    explicit SchedulerController(SchedulerService& schedulerService);
    void registerRoutes(httplib::Server& server) const;

   private:
    SchedulerService& schedulerService_;
};
