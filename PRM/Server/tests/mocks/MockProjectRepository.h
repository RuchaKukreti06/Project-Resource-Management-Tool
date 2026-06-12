#pragma once

#include <string>
#include <vector>

#include "Milestones.h"
#include "Project.h"
#include "repositories/IProjectRepository.h"

class MockProjectRepository : public IProjectRepository
{
   public:
    Project project;
    std::vector<Project> projects;

    Project getProjectById(int) override
    {
        return project;
    }

    std::vector<Project> getAllProjects() override
    {
        return projects;
    }

    std::vector<Project> getProjectsByManager(int) override
    {
        return {};
    }

    bool createProject(const Project&) override
    {
        return true;
    }

    bool updateProject(const Project&) override
    {
        return true;
    }

    bool addMilestone(const Milestone&) override
    {
        return true;
    }

    bool updateMilestoneStatus(int, const std::string&) override
    {
        return true;
    }

    std::vector<Milestone> getMilestonesByProject(int) override
    {
        return {};
    }

    bool updateProjectHealth(int, const std::string&) override
    {
        return true;
    }
};
