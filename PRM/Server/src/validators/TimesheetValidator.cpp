#include "validators/TimesheetValidator.h"

bool TimesheetValidator::validateSubmit(int employeeId,
                                        const std::string& weekStartDate,
                                        const std::vector<TimesheetLineInput>& lines,
                                        int maxWeeklyHours,
                                        std::string& message) const
{
    if (employeeId <= 0)
    {
        message = "Invalid employee id.";
        return false;
    }
    if (weekStartDate.empty())
    {
        message = "Week start date is required.";
        return false;
    }
    if (lines.empty())
    {
        message = "Timesheet must contain at least one line.";
        return false;
    }
    if (maxWeeklyHours <= 0)
    {
        message = "Max weekly hours must be positive.";
        return false;
    }
    return true;
}
