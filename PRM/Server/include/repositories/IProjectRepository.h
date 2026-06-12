#pragma once

#include <string>
#include <vector>

#include "models/Milestones.h"
#include "models/Project.h"

class IProjectRepository
{
   public:
    virtual ~IProjectRepository() = default;

    virtual bool createProject(const Project& project) = 0;
    virtual bool updateProject(const Project& project) = 0;
    virtual Project getProjectById(int id) = 0;
    virtual std::vector<Project> getAllProjects() = 0;
    virtual std::vector<Project> getProjectsByManager(int managerUserId) = 0;

    virtual bool addMilestone(const Milestone& milestone) = 0;
    virtual bool updateMilestoneStatus(int milestoneId, const std::string& status) = 0;
    virtual std::vector<Milestone> getMilestonesByProject(int projectId) = 0;
    virtual bool updateProjectHealth(int projectId, const std::string& health) = 0;
};
