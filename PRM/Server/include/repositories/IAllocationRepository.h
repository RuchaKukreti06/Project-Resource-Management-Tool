#pragma once

#include <string>
#include <vector>
#include <optional>

#include "models/Allocations.h"

class IAllocationRepository
{
   public:
    virtual ~IAllocationRepository() = default;

    virtual bool createAllocation(const Allocation& allocation, int createdByUserId) = 0;
    virtual bool endAllocation(int allocationId, const std::string& endDate) = 0;
    virtual bool endActiveAllocationsByEmployee(int employeeId, const std::string& endDate) = 0;
    virtual std::vector<Allocation> getActiveAllocationsByProject(int projectId) = 0;
    virtual std::vector<Allocation> getOverlappingAllocations(int employeeId, const std::string& fromDate,
                                                               const std::string& toDate) = 0;
    virtual int getOverlappingUtilization(int employeeId, const std::string& fromDate,
                                          const std::string& toDate) = 0;
    virtual int getCurrentUtilization(int employeeId, const std::string& date) = 0;
    virtual std::vector<Allocation> getAllocationsByEmployee(int employeeId) = 0;
    virtual std::optional<Allocation> getAllocationById(int allocationId) = 0;
};
