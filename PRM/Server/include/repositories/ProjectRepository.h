#pragma once

#include "database/Database.h"
#include "repositories/IProjectRepository.h"

class ProjectRepository : public IProjectRepository
{
   public:
    explicit ProjectRepository(database::Database& database);

    bool createProject(const Project& project) override;
    bool updateProject(const Project& project) override;
    Project getProjectById(int id) override;
    std::vector<Project> getAllProjects() override;
    std::vector<Project> getProjectsByManager(int managerUserId) override;

    bool addMilestone(const Milestone& milestone) override;
    bool updateMilestoneStatus(int milestoneId, const std::string& status) override;
    std::vector<Milestone> getMilestonesByProject(int projectId) override;
    bool updateProjectHealth(int projectId, const std::string& health) override;

   private:
    database::Database& database_;
};
