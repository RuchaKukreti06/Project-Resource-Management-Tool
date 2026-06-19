#pragma once

#include "services/interfaces/IProjectService.h"
#include "httplib.h"

class ProjectController
{
   public:
    explicit ProjectController(IProjectService& projectService);
    void registerRoutes(httplib::Server& server);

   private:
    IProjectService& projectService_;

    void handleGetAllProjects(const httplib::Request& req, httplib::Response& res);
    void handleGetProjectById(const httplib::Request& req, httplib::Response& res);
    void handleGetManagerProjects(const httplib::Request& req, httplib::Response& res);
    void handleCreateProject(const httplib::Request& req, httplib::Response& res);
    void handleUpdateProject(const httplib::Request& req, httplib::Response& res);
    void handleGetProjectMilestones(const httplib::Request& req, httplib::Response& res);
    void handleAddMilestone(const httplib::Request& req, httplib::Response& res);
    void handleUpdateMilestoneStatus(const httplib::Request& req, httplib::Response& res);
};
