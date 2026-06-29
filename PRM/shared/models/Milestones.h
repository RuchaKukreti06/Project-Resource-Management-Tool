#ifndef MILESTONES_H
#define MILESTONES_H

#include <string>

struct Milestone
{
    int         id          = 0;
    int         projectId   = 0;
    std::string title;
    std::string dueDate;
    int         storyPoints = 0;
    std::string status;      // NOT_STARTED, IN_PROGRESS, DONE
    std::string healthFlag;  // NORMAL, OVERDUE
};

#endif