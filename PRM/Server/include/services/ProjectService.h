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

    bool createProject(const ProjectCreateRequest& req, std::string& message) override;
    std::optional<ProjectResponse> getProjectById(int projectId) override;
    std::vector<ProjectResponse> getAllProjects() override;
    std::vector<ProjectResponse> getManagerProjects(int managerUserId) override;
    bool updateProject(const UpdateProjectRequest& req, std::string& message) override;
    bool addMilestone(const AddMilestoneRequest& req, std::string& message) override;
    bool updateMilestoneStatus(const UpdateMilestoneStatusRequest& req, std::string& message) override;
    std::vector<MilestoneResponse> getProjectMilestones(int projectId) override;
    bool updateProjectHealth(int projectId, const std::string& health) override;

   private:
    bool isValidManager(int managerUserId) const;

    std::shared_ptr<IProjectRepository> projectRepository_;
    std::shared_ptr<IUserRepository>    userRepository_;
    ProjectValidator                    projectValidator_;
};
