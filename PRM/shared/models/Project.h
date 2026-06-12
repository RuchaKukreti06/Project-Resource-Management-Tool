#ifndef PROJECT_H
#define PROJECT_H

#include <string>

struct Project
{
    int         id                = 0;
    std::string name;
    std::string description;
    std::string startDate;
    std::string endDate;
    int         totalStoryPoints  = 0;
    std::string status;           // PLANNED, ACTIVE, ON_HOLD, COMPLETED
    std::string healthStatus;     // ON_TRACK, ATTENTION, AT_RISK
    int         managerId         = 0;
    std::string createdAt;
    std::string updatedAt;
};

#endif