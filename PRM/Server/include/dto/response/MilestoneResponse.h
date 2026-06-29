#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct MilestoneResponse
{
    int id;
    int projectId;
    std::string title;
    std::string dueDate;
    int storyPoints;
    std::string status;
    std::string healthFlag;
};

inline void to_json(nlohmann::json& j, const MilestoneResponse& dto)
{
    j = nlohmann::json{
        {"id", dto.id},
        {"project_id", dto.projectId},
        {"title", dto.title},
        {"due_date", dto.dueDate},
        {"story_points", dto.storyPoints},
        {"status", dto.status},
        {"health_flag", dto.healthFlag}
    };
}
