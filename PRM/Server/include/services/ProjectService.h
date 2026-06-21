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

    void createProject(const ProjectCreateRequest& req) override;
    std::optional<ProjectResponse> getProjectById(int projectId) override;
    std::vector<ProjectResponse> getAllProjects() override;
    std::vector<ProjectResponse> getManagerProjects(int managerUserId) override;
    void updateProject(const UpdateProjectRequest& req) override;
    void addMilestone(const AddMilestoneRequest& req) override;
    void updateMilestoneStatus(const UpdateMilestoneStatusRequest& req) override;
    std::vector<MilestoneResponse> getProjectMilestones(int projectId) override;
    void updateProjectHealth(int projectId, const std::string& health) override;
    std::string computeProjectHealth(int projectId, const std::string& todayDate) override;
    void recomputeProjectHealth(const std::string& todayDate) override;

   private:
    bool isValidManager(int managerUserId) const;

    std::shared_ptr<IProjectRepository> projectRepository_;
    std::shared_ptr<IUserRepository>    userRepository_;
    ProjectValidator                    projectValidator_;
};
