#include "ProjectController.h"

#include <nlohmann/json.hpp>

namespace
{

nlohmann::json makeResponse(bool success, const std::string& message)
{
    return {{"success", success}, {"message", message}};
}

// D8: Include health_status and total_story_points
nlohmann::json projectToJson(const Project& project)
{
    return {{"id",                  project.id},
            {"name",                project.name},
            {"description",         project.description},
            {"start_date",          project.startDate},
            {"end_date",            project.endDate},
            {"total_story_points",  project.totalStoryPoints},
            {"status",              project.status},
            {"health_status",       project.healthStatus},
            {"manager_id",          project.managerId},
            {"created_at",          project.createdAt}};
}

// D9: Include story_points and health_flag
nlohmann::json milestoneToJson(const Milestone& milestone)
{
    return {{"id",           milestone.id},
            {"project_id",   milestone.projectId},
            {"title",        milestone.title},
            {"due_date",     milestone.dueDate},
            {"story_points", milestone.storyPoints},
            {"status",       milestone.status},
            {"health_flag",  milestone.healthFlag}};
}

}  // namespace

ProjectController::ProjectController(IProjectService& projectService)
    : projectService_(projectService)
{
}

void ProjectController::registerRoutes(httplib::Server& server)
{
    server.Get("/projects", [this](const httplib::Request& req, httplib::Response& res)
               { handleGetAllProjects(req, res); });

    server.Get(R"(/projects/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
               { handleGetProjectById(req, res); });

    server.Get(R"(/managers/(\d+)/projects)",
               [this](const httplib::Request& req, httplib::Response& res)
               { handleGetManagerProjects(req, res); });

    server.Post("/projects", [this](const httplib::Request& req, httplib::Response& res)
                { handleCreateProject(req, res); });

    server.Put(R"(/projects/(\d+))", [this](const httplib::Request& req, httplib::Response& res)
               { handleUpdateProject(req, res); });

    server.Get(R"(/projects/(\d+)/milestones)",
               [this](const httplib::Request& req, httplib::Response& res)
               { handleGetProjectMilestones(req, res); });

    server.Post(R"(/projects/(\d+)/milestones)",
                [this](const httplib::Request& req, httplib::Response& res)
                { handleAddMilestone(req, res); });

    server.Put(R"(/milestones/(\d+)/status)",
               [this](const httplib::Request& req, httplib::Response& res)
               { handleUpdateMilestoneStatus(req, res); });
}

void ProjectController::handleGetAllProjects(const httplib::Request&, httplib::Response& res)
{
    try
    {
        const auto projects = projectService_.getAllProjects();

        nlohmann::json data = nlohmann::json::array();
        for (const auto& p : projects) data.push_back(projectToJson(p));

        res.status = 200;
        res.set_content(nlohmann::json({{"success", true}, {"data", data}}).dump(),
                        "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 500;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void ProjectController::handleGetProjectById(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const int projectId = std::stoi(req.matches[1]);

        auto project = projectService_.getProjectById(projectId);

        if (!project.has_value())
        {
            res.status = 404;
            res.set_content(makeResponse(false, "Project not found.").dump(), "application/json");
            return;
        }

        res.status = 200;
        res.set_content(nlohmann::json{{"success", true}, {"data", projectToJson(*project)}}.dump(),
                        "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 500;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void ProjectController::handleGetManagerProjects(const httplib::Request& req,
                                                 httplib::Response& res)
{
    try
    {
        const int managerId = std::stoi(req.matches[1]);
        const auto projects = projectService_.getManagerProjects(managerId);

        nlohmann::json data = nlohmann::json::array();
        for (const auto& p : projects) data.push_back(projectToJson(p));

        res.status = 200;
        res.set_content(nlohmann::json({{"success", true}, {"data", data}}).dump(),
                        "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 500;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void ProjectController::handleCreateProject(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const auto body = nlohmann::json::parse(req.body);

        Project project;
        project.name             = body.at("name").get<std::string>();
        project.description      = body.value("description", "");
        project.startDate        = body.value("start_date", "");
        project.endDate          = body.value("end_date", "");
        project.totalStoryPoints = body.value("total_story_points", 0);  // D10
        project.status           = body.value("status", "PLANNED");
        project.healthStatus     = body.value("health_status", "ON_TRACK");
        project.managerId        = body.at("manager_id").get<int>();

        std::string message;
        const bool ok = projectService_.createProject(project, message);

        res.status = ok ? 201 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void ProjectController::handleUpdateProject(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const int projectId = std::stoi(req.matches[1]);
        const auto body = nlohmann::json::parse(req.body);

        Project project;
        project.id               = projectId;
        project.name             = body.at("name").get<std::string>();
        project.description      = body.value("description", "");
        project.startDate        = body.value("start_date", "");
        project.endDate          = body.value("end_date", "");
        project.totalStoryPoints = body.value("total_story_points", 0);  // D10
        project.status           = body.at("status").get<std::string>();
        project.healthStatus     = body.value("health_status", "ON_TRACK");
        project.managerId        = body.at("manager_id").get<int>();

        std::string message;
        const bool ok = projectService_.updateProject(project, message);

        res.status = ok ? 200 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void ProjectController::handleGetProjectMilestones(const httplib::Request& req,
                                                   httplib::Response& res)
{
    try
    {
        const int projectId = std::stoi(req.matches[1]);
        const auto milestones = projectService_.getProjectMilestones(projectId);

        nlohmann::json data = nlohmann::json::array();
        for (const auto& m : milestones) data.push_back(milestoneToJson(m));

        res.status = 200;
        res.set_content(nlohmann::json({{"success", true}, {"data", data}}).dump(),
                        "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 500;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void ProjectController::handleAddMilestone(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        const int projectId = std::stoi(req.matches[1]);
        const auto body = nlohmann::json::parse(req.body);

        Milestone milestone;
        milestone.projectId   = projectId;
        milestone.title       = body.at("title").get<std::string>();
        milestone.dueDate     = body.value("due_date", "");
        milestone.storyPoints = body.value("story_points", 0);  // D9/D10
        milestone.status      = body.value("status", "NOT_STARTED");
        milestone.healthFlag  = body.value("health_flag", "NORMAL");

        std::string message;
        const bool ok = projectService_.addMilestone(milestone, message);

        res.status = ok ? 201 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}

void ProjectController::handleUpdateMilestoneStatus(const httplib::Request& req,
                                                    httplib::Response& res)
{
    try
    {
        const int milestoneId = std::stoi(req.matches[1]);
        const auto body = nlohmann::json::parse(req.body);
        const auto status = body.at("status").get<std::string>();

        std::string message;
        const bool ok = projectService_.updateMilestoneStatus(milestoneId, status, message);

        res.status = ok ? 200 : 400;
        res.set_content(makeResponse(ok, message).dump(), "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(makeResponse(false, e.what()).dump(), "application/json");
    }
}
