#pragma once

#include <vector>
#include <optional>
#include <string>
#include "models/Project.h"
#include "models/Milestones.h"

#include "dto/request/ProjectCreateRequest.h"
#include "dto/request/UpdateProjectRequest.h"
#include "dto/request/AddMilestoneRequest.h"
#include "dto/request/UpdateProjectRequest.h"
#include "dto/request/UpdateMilestoneStatusRequest.h"
#include "dto/response/ProjectResponse.h"
#include "dto/response/MilestoneResponse.h"

class IProjectService {
public:
    virtual ~IProjectService() = default;
    virtual bool createProject(const ProjectCreateRequest& req, std::string& message) = 0;
    virtual std::optional<ProjectResponse> getProjectById(int projectId) = 0;
    virtual std::vector<ProjectResponse> getAllProjects() = 0;
    virtual std::vector<ProjectResponse> getManagerProjects(int managerUserId) = 0;
    virtual bool updateProject(const UpdateProjectRequest& req, std::string& message) = 0;
    virtual bool addMilestone(const AddMilestoneRequest& req, std::string& message) = 0;
    virtual bool updateMilestoneStatus(const UpdateMilestoneStatusRequest& req, std::string& message) = 0;
    virtual std::vector<MilestoneResponse> getProjectMilestones(int projectId) = 0;
    virtual bool updateProjectHealth(int projectId, const std::string& health) = 0;
};
