#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct TimesheetResponse
{
    int id;
    int employeeId;
    std::string weekStartDate;
    std::string submittedAt;
    std::string status;
    int totalHours;
};

inline void to_json(nlohmann::json& j, const TimesheetResponse& dto)
{
    j = nlohmann::json{
        {"id", dto.id},
        {"employee_id", dto.employeeId},
        {"week_start_date", dto.weekStartDate},
        {"submitted_at", dto.submittedAt},
        {"status", dto.status},
        {"total_hours", dto.totalHours}
    };
}
