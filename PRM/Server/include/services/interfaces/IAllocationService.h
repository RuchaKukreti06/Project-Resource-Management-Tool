#pragma once

#include <vector>
#include <string>
#include <optional>
#include "models/Allocations.h"

#include "dto/request/AllocationCreateRequest.h"
#include "dto/request/EndAllocationRequest.h"
#include "dto/response/AllocationResponse.h"

class IAllocationService {
public:
    virtual ~IAllocationService() = default;
    virtual void createAllocation(const AllocationCreateRequest& req, int createdByUserId) = 0;
    virtual void endAllocation(const EndAllocationRequest& req) = 0;
    virtual std::vector<AllocationResponse> getProjectAllocations(int projectId) = 0;
    virtual std::vector<AllocationResponse> getEmployeeAllocations(int employeeId) = 0;
    virtual void recomputeEmployeeStatus(int employeeId, const std::string& todayDate) = 0;
    virtual std::optional<AllocationResponse> getAllocationById(int allocationId) = 0;
};
