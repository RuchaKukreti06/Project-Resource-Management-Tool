#pragma once

#include "httplib.h"
#include "services/interfaces/IAllocationService.h"

class AllocationController
{
   public:
    explicit AllocationController(IAllocationService& allocationService);
    void registerRoutes(httplib::Server& server);
    void handleCreateAllocation(const httplib::Request& req, httplib::Response& res);
    void handleEndAllocation(const httplib::Request& req, httplib::Response& res);
    void handleGetProjectAllocations(const httplib::Request& req, httplib::Response& res);

   private:
    IAllocationService& allocationService_;
};
