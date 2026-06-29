#ifndef TIMESHEET_LINE_TAGS_H
#define TIMESHEET_LINE_TAGS_H

#include <string>

struct TimesheetLineTag
{
    int id = 0;
    int timesheetLineId = 0;
    std::string activityTag;
};

#endif