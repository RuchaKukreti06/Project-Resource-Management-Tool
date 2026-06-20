#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct TimesheetDetailResponse
{
    int projectId;
    std::string projectName;
    int hours;
    std::string tags;
};

inline void to_json(nlohmann::json& j, const TimesheetDetailResponse& dto)
{
    j = nlohmann::json{
        {"project_id", dto.projectId},
        {"project_name", dto.projectName},
        {"hours", dto.hours},
        {"tags", dto.tags}
    };
}
