#pragma once

#include "httplib.h"
#include "services/AllocationService.h"

class AllocationController
{
   public:
    explicit AllocationController(AllocationService& allocationService);
    void registerRoutes(httplib::Server& server);
    void handleCreateAllocation(const httplib::Request& req, httplib::Response& res);
    void handleEndAllocation(const httplib::Request& req, httplib::Response& res);
    void handleGetProjectAllocations(const httplib::Request& req, httplib::Response& res);

   private:
    AllocationService& allocationService_;
};
