#pragma once
#include <string>

struct AddMilestoneRequest
{
    int projectId;
    std::string title;
    std::string dueDate;
    int storyPoints;
    std::string status;
    std::string healthFlag;
};
