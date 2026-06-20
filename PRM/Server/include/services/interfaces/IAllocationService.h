#pragma once

#include <vector>
#include <string>
#include "models/Allocations.h"

#include "dto/request/AllocationCreateRequest.h"
#include "dto/request/EndAllocationRequest.h"
#include "dto/response/AllocationResponse.h"

class IAllocationService {
public:
    virtual ~IAllocationService() = default;
    virtual bool createAllocation(const AllocationCreateRequest& req, int createdByUserId, std::string& message) = 0;
    virtual bool endAllocation(const EndAllocationRequest& req, std::string& message) = 0;
    virtual std::vector<AllocationResponse> getProjectAllocations(int projectId) = 0;
    virtual bool recomputeEmployeeStatus(int employeeId, const std::string& todayDate) = 0;
};
