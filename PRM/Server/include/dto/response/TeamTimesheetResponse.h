#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct TeamTimesheetResponse
{
    std::string employeeName;
    int employeeId;
    int projectId;
    std::string projectName;
    int hours;
    std::string status;
};

inline void to_json(nlohmann::json& j, const TeamTimesheetResponse& dto)
{
    j = nlohmann::json{
        {"employee_name", dto.employeeName},
        {"employee_id", dto.employeeId},
        {"project_id", dto.projectId},
        {"project_name", dto.projectName},
        {"hours", dto.hours},
        {"status", dto.status}
    };
}
