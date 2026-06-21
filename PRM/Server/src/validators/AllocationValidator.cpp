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
    if (allocation.fromDate.length() != 10 || allocation.toDate.length() != 10)
    {
        message = "Dates must be in YYYY-MM-DD format.";
        return false;
    }
    return true;
}
