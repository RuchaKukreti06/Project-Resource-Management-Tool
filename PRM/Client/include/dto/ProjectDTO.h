#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct MilestoneDTO
{
    int id;
    std::string title;
    std::string dueDate;
    std::string status;

    static MilestoneDTO fromJson(const nlohmann::json& j)
    {
        MilestoneDTO m;
        m.id = j.value("id", 0);
        m.title = j.value("title", "");
        m.dueDate = j.value("due_date", "");
        m.status = j.value("status", "");
        return m;
    }
};

struct ProjectDTO
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

    static ProjectDTO fromJson(const nlohmann::json& j)
    {
        ProjectDTO p;
        p.id = j.value("id", 0);
        p.name = j.value("name", "");
        p.description = j.value("description", "");
        p.startDate = j.value("start_date", "");
        p.endDate = j.value("end_date", "");
        p.totalStoryPoints = j.value("total_story_points", 0);
        p.status = j.value("status", "");
        p.healthStatus = j.value("health_status", "");
        p.managerId = j.value("manager_id", 0);
        return p;
    }
};
