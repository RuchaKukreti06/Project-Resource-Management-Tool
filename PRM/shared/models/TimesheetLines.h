#ifndef TIMESHEET_LINES_H
#define TIMESHEET_LINES_H

#include <string>

struct TimesheetLine
{
    int id = 0;
    int timesheetId = 0;
    int projectId = 0;
    double hoursWorked = 0.0;
};

#endif