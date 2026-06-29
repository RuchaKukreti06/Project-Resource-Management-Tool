#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct ProjectResponse
{
    int id;
    std::string name;
    std::string description;
    std::string startDate;
    std::string endDate;
    int totalStoryPoints;
    std::string status;
    std::string healthStatus;
    int managerId;
    std::string createdAt;
};

inline void to_json(nlohmann::json& j, const ProjectResponse& dto)
{
    j = nlohmann::json{
        {"id", dto.id},
        {"name", dto.name},
        {"description", dto.description},
        {"start_date", dto.startDate},
        {"end_date", dto.endDate},
        {"total_story_points", dto.totalStoryPoints},
        {"status", dto.status},
        {"health_status", dto.healthStatus},
        {"manager_id", dto.managerId},
        {"created_at", dto.createdAt}
    };
}
