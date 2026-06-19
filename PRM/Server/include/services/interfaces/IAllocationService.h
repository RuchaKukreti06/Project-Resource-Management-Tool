#pragma once

#include <vector>
#include <string>
#include "models/Allocations.h"

class IAllocationService {
public:
    virtual ~IAllocationService() = default;
    virtual bool createAllocation(const Allocation& allocation, int createdByUserId, std::string& message) = 0;
    virtual bool endAllocation(int allocationId, const std::string& endDate, std::string& message) = 0;
    virtual std::vector<Allocation> getProjectAllocations(int projectId) = 0;
    virtual bool recomputeEmployeeStatus(int employeeId, const std::string& todayDate) = 0;
};
