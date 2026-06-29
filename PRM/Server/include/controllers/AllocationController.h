#pragma once

#include "httplib.h"
#include "services/interfaces/IAllocationService.h"
#include "services/interfaces/ITokenService.h"
#include "services/interfaces/IEmployeeService.h"

#include "services/interfaces/IProjectService.h"

class AllocationController
{
   public:
    explicit AllocationController(IAllocationService& allocationService, ITokenService& tokenService, IEmployeeService& employeeService, IProjectService& projectService);
    void registerRoutes(httplib::Server& server);
    void handleCreateAllocation(const httplib::Request& req, httplib::Response& res);
    void handleEndAllocation(const httplib::Request& req, httplib::Response& res);
    void handleGetProjectAllocations(const httplib::Request& req, httplib::Response& res);
    void handleGetEmployeeAllocations(const httplib::Request& req, httplib::Response& res);

   private:
    IAllocationService& allocationService_;
    ITokenService& tokenService_;
    IEmployeeService& employeeService_;
    IProjectService& projectService_;
};
