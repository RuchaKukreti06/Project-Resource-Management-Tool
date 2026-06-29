#pragma once
#include <string>

struct UpdateMilestoneStatusRequest
{
    int milestoneId;
    std::string status;
};
