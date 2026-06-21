#include "services/ProjectService.h"
#include "dto/DTOMapper.h"
#include "exceptions/Exceptions.h"
#include <spdlog/spdlog.h>
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

void ProjectService::createProject(const ProjectCreateRequest& req)
{
    if (!isValidManager(req.managerId))
    {
        throw exceptions::ValidationException("Invalid manager id.");
    }

    if (req.totalStoryPoints < 0 || req.totalStoryPoints > 10000)
    {
        throw exceptions::ValidationException("Total story points must be between 0 and 10000.");
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

    if (!projectRepository_->createProject(created))
    {
        throw exceptions::DatabaseException("Failed to create project.");
    }
}

void ProjectService::updateProject(const UpdateProjectRequest& req)
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
        throw exceptions::ValidationException("Invalid manager id.");
    }

    if (project.totalStoryPoints < 0 || project.totalStoryPoints > 10000)
    {
        throw exceptions::ValidationException("Total story points must be between 0 and 10000.");
    }

    if (!projectRepository_->updateProject(project))
    {
        throw exceptions::DatabaseException("Failed to update project.");
    }
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

void ProjectService::addMilestone(const AddMilestoneRequest& req)
{
    Milestone createInput;
    createInput.projectId = req.projectId;
    createInput.title = req.title;
    createInput.dueDate = req.dueDate;
    createInput.storyPoints = req.storyPoints;
    createInput.status = req.status.empty() ? "NOT_STARTED" : req.status;
    createInput.healthFlag = req.healthFlag.empty() ? "NORMAL" : req.healthFlag;

    std::string message;
    if (!projectValidator_.validateMilestoneStatus(createInput.status, message))
    {
        throw exceptions::ValidationException(message);
    }

    if (!projectRepository_->addMilestone(createInput))
    {
        throw exceptions::DatabaseException("Failed to add milestone.");
    }
}

void ProjectService::updateMilestoneStatus(const UpdateMilestoneStatusRequest& req)
{
    std::string message;
    if (!projectValidator_.validateMilestoneStatus(req.status, message))
    {
        throw exceptions::ValidationException(message);
    }

    if (!projectRepository_->updateMilestoneStatus(req.milestoneId, req.status))
    {
        throw exceptions::DatabaseException("Failed to update milestone.");
    }
}

std::vector<MilestoneResponse> ProjectService::getProjectMilestones(int projectId)
{
    auto milestones = projectRepository_->getMilestonesByProject(projectId);
    return DTOMapper::mapToMilestoneResponse(milestones);
}

void ProjectService::updateProjectHealth(int projectId, const std::string& health)
{
    if (!projectRepository_->updateProjectHealth(projectId, health))
    {
        throw exceptions::DatabaseException("Failed to update project health.");
    }
}

std::string ProjectService::computeProjectHealth(int projectId, const std::string& todayDate)
{
    const auto milestones = getProjectMilestones(projectId);

    bool hasOverdue     = false;
    bool hasInProgress  = false;

    for (const auto& m : milestones)
    {
        if (m.status == "DONE")
            continue;

        if (m.dueDate < todayDate)
        {
            hasOverdue = true;
        }
        if (m.status == "IN_PROGRESS")
        {
            hasInProgress = true;
        }
    }

    if (hasOverdue)
        return "AT_RISK";
    if (hasInProgress)
        return "ATTENTION";
    return "ON_TRACK";
}

void ProjectService::recomputeProjectHealth(const std::string& todayDate)
{
    spdlog::info("ProjectService: recomputing project health for {}", todayDate);
    const auto projects = getAllProjects();

    for (const auto& project : projects)
    {
        if (project.status == "COMPLETED" || project.status == "ON_HOLD")
            continue;

        const std::string health = computeProjectHealth(project.id, todayDate);
        if (health != project.healthStatus)
        {
            try
            {
                updateProjectHealth(project.id, health);
                spdlog::info("ProjectService: project {} health updated to {}", project.id, health);
            }
            catch (const std::exception& e)
            {
                spdlog::error("ProjectService: Failed to update project {} health: {}", project.id, e.what());
            }
        }
    }
}
