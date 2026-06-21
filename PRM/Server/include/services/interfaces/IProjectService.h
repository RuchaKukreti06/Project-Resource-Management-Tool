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
    virtual void createProject(const ProjectCreateRequest& req) = 0;
    virtual std::optional<ProjectResponse> getProjectById(int projectId) = 0;
    virtual std::vector<ProjectResponse> getAllProjects() = 0;
    virtual std::vector<ProjectResponse> getManagerProjects(int managerUserId) = 0;
    virtual void updateProject(const UpdateProjectRequest& req) = 0;
    virtual void addMilestone(const AddMilestoneRequest& req) = 0;
    virtual void updateMilestoneStatus(const UpdateMilestoneStatusRequest& req) = 0;
    virtual std::vector<MilestoneResponse> getProjectMilestones(int projectId) = 0;
    virtual void updateProjectHealth(int projectId, const std::string& health) = 0;
    virtual std::string computeProjectHealth(int projectId, const std::string& todayDate) = 0;
    virtual void recomputeProjectHealth(const std::string& todayDate) = 0;
};
