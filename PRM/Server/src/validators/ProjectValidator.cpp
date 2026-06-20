#include "validators/ProjectValidator.h"

#include <unordered_set>

bool ProjectValidator::validateMilestoneStatus(const std::string& status,
                                               std::string& message) const
{
    static const std::unordered_set<std::string> validStatus = {
        "NOT_STARTED", "IN_PROGRESS", "DONE"};

    if (validStatus.find(status) == validStatus.end())
    {
        message = "Invalid milestone status.";
        return false;
    }
    return true;
}
