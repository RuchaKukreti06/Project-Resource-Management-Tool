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

struct CreateProjectRequest {
    std::string name;
    std::string description;
    std::string startDate;
    std::string endDate;
    std::string status;
    int totalStoryPoints;
    std::string healthStatus;
    int managerId;

    nlohmann::json toJson() const {
        return {
            {"name", name},
            {"description", description},
            {"start_date", startDate},
            {"end_date", endDate},
            {"status", status},
            {"total_story_points", totalStoryPoints},
            {"health_status", healthStatus},
            {"manager_id", managerId}
        };
    }
};

struct AddMilestoneRequest {
    std::string title;
    std::string dueDate;
    int storyPoints;
    std::string status;
    std::string healthFlag;

    nlohmann::json toJson() const {
        return {
            {"title", title},
            {"due_date", dueDate},
            {"story_points", storyPoints},
            {"status", status},
            {"health_flag", healthFlag}
        };
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
