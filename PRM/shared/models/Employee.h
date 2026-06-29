#ifndef EMPLOYEE_H
#define EMPLOYEE_H
#include <string>

struct Employee
{
    int         id                = 0;
    int         user_id           = 0;
    int         manager_id        = 0;
    std::string fullName;
    std::string email;
    std::string department;
    std::string designation;
    std::string status;            // BENCH, ALLOCATED
    bool        isActive          = true;
    int         totalUtilisation  = 0;  // D7: from resources.total_utilisation
    std::string createdAt;
};

#endif
