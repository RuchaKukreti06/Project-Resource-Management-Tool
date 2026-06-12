#ifndef ALLOCATIONS_H
#define ALLOCATIONS_H

#include <string>

struct Allocation
{
    int id = 0;
    int employeeId = 0;
    int projectId = 0;
    int utilizationPercentage = 0;
    std::string fromDate;
    std::string toDate;
    std::string status;
    std::string createdAt;
};

#endif