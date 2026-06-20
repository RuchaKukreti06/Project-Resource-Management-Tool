#include "services/ProjectService.h"
#include "dto/DTOMapper.h"

#include <unordered_set>

ProjectService::ProjectService(std::shared_ptr<IProjectRepository> projectRepository,
                               std::shared_ptr<IUserRepository> userRepository)
    : projectRepository_(std::move(projectRepository)), userRepository_(std::move(userRepository))
{
}

bool ProjectService::isValidManager(int managerUserId) const
{
    const auto manager = userRepository_->getUserById(managerUserId);
    return manager.id != 0 && (manager.role == "MANAGER" || manager.role == "ADMIN") &&
           manager.status == "ACTIVE";
}

bool ProjectService::createProject(const ProjectCreateRequest& req, std::string& message)
{
    if (!isValidManager(req.managerId))
    {
        message = "Invalid manager id.";
        return false;
    }

    Project created;
    created.name = req.name;
    created.description = req.description;
    created.startDate = req.startDate;
    created.endDate = req.endDate;
    created.totalStoryPoints = req.totalStoryPoints;
    created.status = req.status.empty() ? "PLANNED" : req.status;
    created.healthStatus = req.healthStatus.empty() ? "ON_TRACK" : req.healthStatus;
    created.managerId = req.managerId;

    const bool ok = projectRepository_->createProject(created);
    message = ok ? "Project created." : "Failed to create project.";
    return ok;
}

bool ProjectService::updateProject(const UpdateProjectRequest& req, std::string& message)
{
    Project project;
    project.id = req.id;
    project.name = req.name;
    project.description = req.description;
    project.startDate = req.startDate;
    project.endDate = req.endDate;
    project.totalStoryPoints = req.totalStoryPoints;
    project.status = req.status;
    project.healthStatus = req.healthStatus.empty() ? "ON_TRACK" : req.healthStatus;
    project.managerId = req.managerId;

    if (!isValidManager(project.managerId))
    {
        message = "Invalid manager id.";
        return false;
    }

    const bool ok = projectRepository_->updateProject(project);
    message = ok ? "Project updated." : "Failed to update project.";
    return ok;
}

std::optional<ProjectResponse> ProjectService::getProjectById(int projectId)
{
    auto p = projectRepository_->getProjectById(projectId);
    if (p.id == 0) return std::nullopt;
    return DTOMapper::mapToProjectResponse(p);
}

std::vector<ProjectResponse> ProjectService::getAllProjects()
{
    return DTOMapper::mapToProjectResponse(projectRepository_->getAllProjects());
}

std::vector<ProjectResponse> ProjectService::getManagerProjects(int managerUserId)
{
    return DTOMapper::mapToProjectResponse(projectRepository_->getProjectsByManager(managerUserId));
}

bool ProjectService::addMilestone(const AddMilestoneRequest& req, std::string& message)
{
    Milestone createInput;
    createInput.projectId = req.projectId;
    createInput.title = req.title;
    createInput.dueDate = req.dueDate;
    createInput.storyPoints = req.storyPoints;
    createInput.status = req.status.empty() ? "NOT_STARTED" : req.status;
    createInput.healthFlag = req.healthFlag.empty() ? "NORMAL" : req.healthFlag;

    if (!projectValidator_.validateMilestoneStatus(createInput.status, message))
    {
        return false;
    }

    const bool ok = projectRepository_->addMilestone(createInput);
    message = ok ? "Milestone added." : "Failed to add milestone.";
    return ok;
}

bool ProjectService::updateMilestoneStatus(const UpdateMilestoneStatusRequest& req, std::string& message)
{
    if (!projectValidator_.validateMilestoneStatus(req.status, message))
    {
        return false;
    }

    const bool ok = projectRepository_->updateMilestoneStatus(req.milestoneId, req.status);
    message = ok ? "Milestone updated." : "Failed to update milestone.";
    return ok;
}

std::vector<MilestoneResponse> ProjectService::getProjectMilestones(int projectId)
{
    auto milestones = projectRepository_->getMilestonesByProject(projectId);
    return DTOMapper::mapToMilestoneResponse(milestones);
}

bool ProjectService::updateProjectHealth(int projectId, const std::string& health)
{
    return projectRepository_->updateProjectHealth(projectId, health);
}
