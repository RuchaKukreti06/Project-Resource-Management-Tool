#pragma once

#include <string>

class ProjectValidator
{
public:
    // Validates that status is one of NOT_STARTED / IN_PROGRESS / DONE.
    bool validateMilestoneStatus(const std::string& status, std::string& message) const;
};
