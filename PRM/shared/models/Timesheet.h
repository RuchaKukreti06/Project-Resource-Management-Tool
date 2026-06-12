#ifndef TIMESHEET_H
#define TIMESHEET_H

#include <string>

struct Timesheet
{
    int id = 0;
    int employeeId = 0;
    std::string weekStartDate;
    std::string submittedAt;
    std::string status;
    int totalHours = 0;
};

#endif