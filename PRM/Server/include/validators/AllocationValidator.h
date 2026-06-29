#pragma once

#include <string>

#include "models/Allocations.h"

class AllocationValidator
{
public:
    // Validates employeeId > 0, projectId > 0, utilizationPercentage in (0,100],
    // fromDate non-empty, and toDate non-empty.
    bool validateCreate(const Allocation& allocation, std::string& message) const;
};
