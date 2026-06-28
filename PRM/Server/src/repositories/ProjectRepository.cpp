#include "repositories/ProjectRepository.h"

#include <mysqlx/xdevapi.h>
#include <spdlog/spdlog.h>

namespace
{

// Strip bytes that are not valid UTF-8 / JSON-safe printable characters.
// Prevents nlohmann::json type_error.316 when DB rows contain control chars
// (e.g. 0x0F inserted by a project name like "Canon\").
std::string sanitizeUtf8(const std::string& raw)
{
    std::string out;
    out.reserve(raw.size());
    for (unsigned char c : raw)
    {
        if (c == 0x09 || c == 0x0A || c == 0x0D)
            out += static_cast<char>(c);          // tab, LF, CR — allowed
        else if (c >= 0x20 && c < 0x7F)
            out += static_cast<char>(c);          // printable ASCII only
        // else: skip control chars and extended/invalid bytes
    }
    return out;
}

// Columns: id, name, description, start_date, end_date, total_story_points,
//          status, health_status, manager_id, created_at
Project mapProjectRow(const mysqlx::Row& row)
{
    Project project;
    project.id              = row[0].get<int>();
    project.name            = sanitizeUtf8(!row[1].isNull() ? row[1].get<std::string>() : "");
    project.description     = sanitizeUtf8(!row[2].isNull() ? row[2].get<std::string>() : "");
    project.startDate       = sanitizeUtf8(!row[3].isNull() ? row[3].get<std::string>() : "");
    project.endDate         = sanitizeUtf8(!row[4].isNull() ? row[4].get<std::string>() : "");
    project.totalStoryPoints = row[5].get<int>();
    project.status          = sanitizeUtf8(row[6].get<std::string>());
    project.healthStatus    = sanitizeUtf8(row[7].get<std::string>());
    project.managerId       = !row[8].isNull() ? row[8].get<int>() : 0;
    project.createdAt       = sanitizeUtf8(!row[9].isNull() ? row[9].get<std::string>() : "");
    return project;
}

// Columns: id, project_id, title, due_date, story_points, status, health_flag
Milestone mapMilestoneRow(const mysqlx::Row& row)
{
    Milestone milestone;
    milestone.id          = row[0].get<int>();
    milestone.projectId   = row[1].get<int>();
    milestone.title       = sanitizeUtf8(!row[2].isNull() ? row[2].get<std::string>() : "");
    milestone.dueDate     = !row[3].isNull() ? row[3].get<std::string>() : "";
    milestone.storyPoints = row[4].get<int>();
    milestone.status      = sanitizeUtf8(row[5].get<std::string>());
    milestone.healthFlag  = sanitizeUtf8(row[6].get<std::string>());
    return milestone;
}

static const char* kSelectProjectSQL =
    "SELECT id, name, description, start_date, end_date, total_story_points, "
    "status, health_status, manager_id, DATE_FORMAT(created_at, '%Y-%m-%d %T') "
    "FROM projects";

}  // namespace


ProjectRepository::ProjectRepository(database::Database& database) : database_(database)
{
}

bool ProjectRepository::createProject(const Project& project)
{
    try
    {
        database_.getSession()
            .sql("INSERT INTO projects "
                 "(name, description, start_date, end_date, total_story_points, status, health_status, manager_id) "
                 "VALUES (?, ?, NULLIF(?, ''), NULLIF(?, ''), ?, ?, ?, ?)")
            .bind(project.name)
            .bind(project.description)
            .bind(project.startDate)
            .bind(project.endDate)
            .bind(project.totalStoryPoints)
            .bind(project.status)
            .bind(project.healthStatus.empty() ? "ON_TRACK" : project.healthStatus)
            .bind(project.managerId)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("createProject failed: {}", e.what());
        return false;
    }
}

bool ProjectRepository::updateProject(const Project& project)
{
    try
    {
        database_.getSession()
            .sql("UPDATE projects SET name=?, description=?, start_date=NULLIF(?, ''), end_date=NULLIF(?, ''), "
                 "total_story_points=?, status=?, health_status=?, manager_id=? WHERE id=?")
            .bind(project.name)
            .bind(project.description)
            .bind(project.startDate)
            .bind(project.endDate)
            .bind(project.totalStoryPoints)
            .bind(project.status)
            .bind(project.healthStatus.empty() ? "ON_TRACK" : project.healthStatus)
            .bind(project.managerId)
            .bind(project.id)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("updateProject failed: {}", e.what());
        return false;
    }
}

Project ProjectRepository::getProjectById(int id)
{
    Project project;
    try
    {
        std::string sql = std::string(kSelectProjectSQL) + " WHERE id = ?";
        auto result = database_.getSession().sql(sql).bind(id).execute();
        if (auto row = result.fetchOne())
        {
            project = mapProjectRow(row);
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getProjectById failed: {}", e.what());
    }
    return project;
}

std::vector<Project> ProjectRepository::getAllProjects()
{
    std::vector<Project> projects;
    try
    {
        auto result = database_.getSession().sql(kSelectProjectSQL).execute();
        while (auto row = result.fetchOne())
        {
            projects.push_back(mapProjectRow(row));
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getAllProjects failed: {}", e.what());
    }
    return projects;
}

std::vector<Project> ProjectRepository::getProjectsByManager(int managerUserId)
{
    std::vector<Project> projects;
    try
    {
        std::string sql = std::string(kSelectProjectSQL) + " WHERE manager_id = ?";
        auto result = database_.getSession().sql(sql).bind(managerUserId).execute();
        while (auto row = result.fetchOne())
        {
            projects.push_back(mapProjectRow(row));
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getProjectsByManager failed: {}", e.what());
    }
    return projects;
}

bool ProjectRepository::addMilestone(const Milestone& milestone)
{
    try
    {
        database_.getSession()
            .sql("INSERT INTO milestones (project_id, title, due_date, story_points, status, health_flag) "
                 "VALUES (?, ?, ?, ?, ?, ?)")
            .bind(milestone.projectId)
            .bind(milestone.title)
            .bind(milestone.dueDate)
            .bind(milestone.storyPoints)
            .bind(milestone.status)
            .bind(milestone.healthFlag.empty() ? "NORMAL" : milestone.healthFlag)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("addMilestone failed: {}", e.what());
        return false;
    }
}

bool ProjectRepository::updateMilestoneStatus(int milestoneId, const std::string& status)
{
    try
    {
        database_.getSession()
            .sql("UPDATE milestones SET status = ? WHERE id = ?")
            .bind(status)
            .bind(milestoneId)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("updateMilestoneStatus failed: {}", e.what());
        return false;
    }
}

std::vector<Milestone> ProjectRepository::getMilestonesByProject(int projectId)
{
    std::vector<Milestone> milestones;
    try
    {
        auto result = database_.getSession()
                          .sql("SELECT id, project_id, title, COALESCE(DATE_FORMAT(due_date, '%Y-%m-%d'), ''), story_points, status, health_flag "
                               "FROM milestones WHERE project_id = ?")
                          .bind(projectId)
                          .execute();
        while (auto row = result.fetchOne())
        {
            milestones.push_back(mapMilestoneRow(row));
        }
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("getMilestonesByProject failed: {}", e.what());
    }
    return milestones;
}

bool ProjectRepository::updateProjectHealth(int projectId, const std::string& healthStatus)
{
    try
    {
        database_.getSession()
            .sql("UPDATE projects SET health_status = ? WHERE id = ?")
            .bind(healthStatus)
            .bind(projectId)
            .execute();
        return true;
    }
    catch (const mysqlx::Error& e)
    {
        spdlog::error("updateProjectHealth failed: {}", e.what());
        return false;
    }
}
