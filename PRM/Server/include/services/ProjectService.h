#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "models/Milestones.h"
#include "models/Project.h"
#include "repositories/IProjectRepository.h"
#include "repositories/IUserRepository.h"

class ProjectService
{
   public:
    ProjectService(std::shared_ptr<IProjectRepository> projectRepository,
                   std::shared_ptr<IUserRepository> userRepository);

    bool createProject(const Project& project, std::string& message);
    bool updateProject(const Project& project, std::string& message);
    std::optional<Project> getProjectById(int projectId);
    std::vector<Project> getAllProjects();
    std::vector<Project> getManagerProjects(int managerUserId);

    bool addMilestone(const Milestone& milestone, std::string& message);
    bool updateMilestoneStatus(int milestoneId, const std::string& status, std::string& message);
    std::vector<Milestone> getProjectMilestones(int projectId);
    bool updateProjectHealth(int projectId, const std::string& health);

   private:
    bool isValidManager(int managerUserId) const;

    std::shared_ptr<IProjectRepository> projectRepository_;
    std::shared_ptr<IUserRepository> userRepository_;
};
