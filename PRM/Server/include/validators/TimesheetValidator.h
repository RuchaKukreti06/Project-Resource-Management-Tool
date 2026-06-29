#pragma once

#include <string>
#include <vector>

#include "repositories/ITimesheetRepository.h"

class TimesheetValidator
{
public:
    // Validates employeeId > 0, weekStartDate non-empty, lines non-empty,
    // and maxWeeklyHours > 0.
    bool validateSubmit(int employeeId,
                        const std::string& weekStartDate,
                        const std::vector<TimesheetLineInput>& lines,
                        int maxWeeklyHours,
                        std::string& message) const;
};
