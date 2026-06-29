#pragma once
#include <string>

struct UpdateProjectRequest
{
    int id;
    std::string name;
    std::string description;
    std::string startDate;
    std::string endDate;
    int totalStoryPoints;
    std::string status;
    std::string healthStatus;
    int managerId;
};
