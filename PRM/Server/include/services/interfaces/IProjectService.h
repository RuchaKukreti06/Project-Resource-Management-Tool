#pragma once

#include <vector>
#include <optional>
#include <string>
#include "models/Project.h"
#include "models/Milestones.h"

class IProjectService {
public:
    virtual ~IProjectService() = default;
    virtual bool createProject(const Project& project, std::string& message) = 0;
    virtual bool updateProject(const Project& project, std::string& message) = 0;
    virtual std::optional<Project> getProjectById(int projectId) = 0;
    virtual std::vector<Project> getAllProjects() = 0;
    virtual std::vector<Project> getManagerProjects(int managerUserId) = 0;
    virtual bool addMilestone(const Milestone& milestone, std::string& message) = 0;
    virtual bool updateMilestoneStatus(int milestoneId, const std::string& status, std::string& message) = 0;
    virtual std::vector<Milestone> getProjectMilestones(int projectId) = 0;
    virtual bool updateProjectHealth(int projectId, const std::string& health) = 0;
};
