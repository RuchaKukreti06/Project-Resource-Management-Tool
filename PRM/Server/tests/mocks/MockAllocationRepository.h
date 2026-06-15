#pragma once

#include <string>
#include <vector>

#include "Allocations.h"
#include "repositories/IAllocationRepository.h"

class MockAllocationRepository : public IAllocationRepository
{
   public:
    bool createResult = true;
    bool endResult = true;
    int overlapUtilization = 0;
    int currentUtilization = 0;

    std::vector<Allocation> allocations;

    bool createAllocation(const Allocation& allocation, int) override
    {
        allocations.push_back(allocation);
        return createResult;
    }

    bool endAllocation(int, const std::string&) override
    {
        return endResult;
    }

    bool endActiveAllocationsByEmployee(int, const std::string&) override
    {
        return true;
    }

    std::vector<Allocation> getActiveAllocationsByProject(int) override
    {
        return allocations;
    }

    std::vector<Allocation> getOverlappingAllocations(int, const std::string&,
                                                      const std::string&) override
    {
        return {};
    }

    int getOverlappingUtilization(int, const std::string&, const std::string&) override
    {
        return overlapUtilization;
    }

    int getCurrentUtilization(int, const std::string&) override
    {
        return currentUtilization;
    }
};
