#pragma once
#include <string>
#include <vector>

struct TimesheetLineDto
{
    int projectId;
    int hoursWorked;
    std::vector<std::string> tags;
};

struct SubmitTimesheetRequest
{
    int employeeId;
    std::string weekStartDate;
    std::vector<TimesheetLineDto> lines;
    int maxWeeklyHours;
};
