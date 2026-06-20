#include "ProjectController.h"

#include <nlohmann/json.hpp>

namespace
{

#include "dto/DTOMapper.h"

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
    const auto projects = projectService_.getAllProjects();

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"data", projects}}).dump(),
                    "application/json");
}

void ProjectController::handleGetProjectById(const httplib::Request& req, httplib::Response& res)
{
    const int projectId = std::stoi(req.matches[1]);

    auto project = projectService_.getProjectById(projectId);

    if (!project.has_value())
    {
        res.status = 404;
        res.set_content(nlohmann::json({{"success", false}, {"message", "Project not found."}}).dump(), "application/json");
        return;
    }

    nlohmann::json response;
    response["success"] = true;
    response["data"] = *project;
    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void ProjectController::handleGetManagerProjects(const httplib::Request& req,
                                                 httplib::Response& res)
{
    const int managerId = std::stoi(req.matches[1]);
    const auto projects = projectService_.getManagerProjects(managerId);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"data", projects}}).dump(),
                    "application/json");
}

void ProjectController::handleCreateProject(const httplib::Request& req, httplib::Response& res)
{
    const auto body = nlohmann::json::parse(req.body);

    ProjectCreateRequest request;
    request.name         = body.at("name").get<std::string>();
    request.description  = body.value("description", "");
    request.startDate    = body.value("start_date", "");
    request.endDate      = body.value("end_date", "");
    request.totalStoryPoints = body.value("total_story_points", 0);
    request.status       = body.value("status", "");
    request.healthStatus = body.value("health_status", "");
    request.managerId    = body.at("manager_id").get<int>();

    projectService_.createProject(request);

    res.status = 201;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Project created."}}).dump(), "application/json");
}

void ProjectController::handleUpdateProject(const httplib::Request& req, httplib::Response& res)
{
    const int projectId = std::stoi(req.matches[1]);
    const auto body = nlohmann::json::parse(req.body);

    UpdateProjectRequest request;
    request.id               = projectId;
    request.name             = body.at("name").get<std::string>();
    request.description      = body.value("description", "");
    request.startDate        = body.value("start_date", "");
    request.endDate          = body.value("end_date", "");
    request.totalStoryPoints = body.value("total_story_points", 0);
    request.status           = body.at("status").get<std::string>();
    request.healthStatus     = body.value("health_status", "");
    request.managerId        = body.at("manager_id").get<int>();

    projectService_.updateProject(request);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Project updated."}}).dump(), "application/json");
}

void ProjectController::handleGetProjectMilestones(const httplib::Request& req,
                                                   httplib::Response& res)
{
    const int projectId = std::stoi(req.matches[1]);
    const auto milestones = projectService_.getProjectMilestones(projectId);

    res.status = 200;
    res.set_content(
        nlohmann::json({{"success", true}, {"data", milestones}}).dump(),
        "application/json");
}

void ProjectController::handleAddMilestone(const httplib::Request& req, httplib::Response& res)
{
    const int projectId = std::stoi(req.matches[1]);
    const auto body = nlohmann::json::parse(req.body);

    AddMilestoneRequest request;
    request.projectId   = projectId;
    request.title       = body.at("title").get<std::string>();
    request.dueDate     = body.value("due_date", "");
    request.storyPoints = body.value("story_points", 0);
    request.status      = body.value("status", "");
    request.healthFlag  = body.value("health_flag", "");

    projectService_.addMilestone(request);

    res.status = 201;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Milestone added."}}).dump(), "application/json");
}

void ProjectController::handleUpdateMilestoneStatus(const httplib::Request& req,
                                                    httplib::Response& res)
{
    const int milestoneId = std::stoi(req.matches[1]);
    const auto body = nlohmann::json::parse(req.body);
    UpdateMilestoneStatusRequest request;
    request.milestoneId = milestoneId;
    request.status = body.at("status").get<std::string>();

    projectService_.updateMilestoneStatus(request);

    res.status = 200;
    res.set_content(nlohmann::json({{"success", true}, {"message", "Milestone updated."}}).dump(), "application/json");
}
