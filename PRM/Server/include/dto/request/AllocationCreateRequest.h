#pragma once
#include <string>

struct AllocationCreateRequest
{
    int employeeId;
    int projectId;
    int utilizationPercentage;
    std::string fromDate;
    std::string toDate;
};
