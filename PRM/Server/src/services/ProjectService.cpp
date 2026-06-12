#include "services/ProjectService.h"

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

bool ProjectService::createProject(const Project& project, std::string& message)
{
    if (!isValidManager(project.managerId))
    {
        message = "Invalid manager id.";
        return false;
    }

    Project created = project;
    if (created.status.empty())
    {
        created.status = "PLANNED";
    }

    const bool ok = projectRepository_->createProject(created);
    message = ok ? "Project created." : "Failed to create project.";
    return ok;
}

bool ProjectService::updateProject(const Project& project, std::string& message)
{
    if (!isValidManager(project.managerId))
    {
        message = "Invalid manager id.";
        return false;
    }

    const bool ok = projectRepository_->updateProject(project);
    message = ok ? "Project updated." : "Failed to update project.";
    return ok;
}

std::optional<Project> ProjectService::getProjectById(int projectId)
{
    auto project = projectRepository_->getProjectById(projectId);
    if (project.id == 0)
    {
        return std::nullopt;
    }
    return project;
}

std::vector<Project> ProjectService::getAllProjects()
{
    return projectRepository_->getAllProjects();
}

std::vector<Project> ProjectService::getManagerProjects(int managerUserId)
{
    return projectRepository_->getProjectsByManager(managerUserId);
}

bool ProjectService::addMilestone(const Milestone& milestone, std::string& message)
{
    static const std::unordered_set<std::string> validStatus = {
        "NOT_STARTED", "IN_PROGRESS", "DONE"};

    Milestone createInput = milestone;
    if (createInput.status.empty())
    {
        createInput.status = "NOT_STARTED";
    }

    if (validStatus.find(createInput.status) == validStatus.end())
    {
        message = "Invalid milestone status.";
        return false;
    }

    const bool ok = projectRepository_->addMilestone(createInput);
    message = ok ? "Milestone added." : "Failed to add milestone.";
    return ok;
}

bool ProjectService::updateMilestoneStatus(int milestoneId, const std::string& status,
                                           std::string& message)
{
    static const std::unordered_set<std::string> validStatus = {
        "NOT_STARTED", "IN_PROGRESS", "DONE"};

    if (validStatus.find(status) == validStatus.end())
    {
        message = "Invalid milestone status.";
        return false;
    }

    const bool ok = projectRepository_->updateMilestoneStatus(milestoneId, status);
    message = ok ? "Milestone updated." : "Failed to update milestone.";
    return ok;
}

std::vector<Milestone> ProjectService::getProjectMilestones(int projectId)
{
    return projectRepository_->getMilestonesByProject(projectId);
}

bool ProjectService::updateProjectHealth(int projectId, const std::string& health)
{
    return projectRepository_->updateProjectHealth(projectId, health);
}
