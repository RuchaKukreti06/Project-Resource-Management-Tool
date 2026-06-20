#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "models/Milestones.h"
#include "models/Project.h"
#include "repositories/IProjectRepository.h"
#include "repositories/IUserRepository.h"
#include "services/interfaces/IProjectService.h"
#include "validators/ProjectValidator.h"

class ProjectService : public IProjectService
{
   public:
    ProjectService(std::shared_ptr<IProjectRepository> projectRepository,
                   std::shared_ptr<IUserRepository> userRepository);

    bool createProject(const Project& project, std::string& message) override;
    bool updateProject(const Project& project, std::string& message) override;
    std::optional<Project> getProjectById(int projectId) override;
    std::vector<Project> getAllProjects() override;
    std::vector<Project> getManagerProjects(int managerUserId) override;

    bool addMilestone(const Milestone& milestone, std::string& message) override;
    bool updateMilestoneStatus(int milestoneId, const std::string& status, std::string& message) override;
    std::vector<Milestone> getProjectMilestones(int projectId) override;
    bool updateProjectHealth(int projectId, const std::string& health) override;

   private:
    bool isValidManager(int managerUserId) const;

    std::shared_ptr<IProjectRepository> projectRepository_;
    std::shared_ptr<IUserRepository>    userRepository_;
    ProjectValidator                    projectValidator_;
};
