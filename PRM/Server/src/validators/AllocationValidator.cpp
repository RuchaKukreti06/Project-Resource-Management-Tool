#include "validators/AllocationValidator.h"

bool AllocationValidator::validateCreate(const Allocation& allocation, std::string& message) const
{
    if (allocation.employeeId <= 0 || allocation.projectId <= 0 ||
        allocation.utilizationPercentage <= 0 || allocation.utilizationPercentage > 100 ||
        allocation.fromDate.empty() || allocation.toDate.empty())
    {
        message = "Invalid allocation payload.";
        return false;
    }
    return true;
}
