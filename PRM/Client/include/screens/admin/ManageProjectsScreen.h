#ifndef MANAGE_PROJECTS_SCREEN_H
#define MANAGE_PROJECTS_SCREEN_H

#include "screens/Screen.h"
#include "dto/ProjectDTO.h"
#include <optional>
#include <string>
#include "admin/adminConstants.h"

class ProjectClientService;

class ManageProjectsScreen : public Screen
{
   public:
    ManageProjectsScreen(ProjectClientService& projService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    void displayProjects(const std::vector<ProjectDTO>& projects);
    void displayMilestones(const std::vector<MilestoneDTO>& milestones);
    std::optional<int> promptForProjectId(const std::string& prompt);
    
    // New validation and prompt helpers
    bool isValidProjectName(const std::string& name);
    bool isValidProjectDateRange(const std::string& start, const std::string& end);
    std::optional<int> promptForStoryPoints(const std::string& prompt);
    std::optional<int> promptForManagerUserId(const std::string& prompt);
    std::optional<std::string> promptForProjectStatus(const std::string& currentStatus = "");
    std::optional<std::string> promptForMilestoneStatus();

    std::optional<std::string> promptForProjectName();
    std::optional<std::pair<std::string, std::string>> promptForProjectDates(const std::string& status);
    
    std::optional<std::pair<std::string, std::string>> promptForUpdatedBasicDetails(const ProjectDTO& currentProj);
    std::optional<std::pair<std::string, std::string>> promptForUpdatedDates(const ProjectDTO& currentProj, const std::string& status);
    std::optional<int> promptForUpdatedManager(const ProjectDTO& currentProj);

    std::optional<CreateProjectRequest> promptForNewProjectDetails();
    std::optional<CreateProjectRequest> promptForProjectUpdate(const ProjectDTO& currentProj);
    
    void handleAddMilestone(int projectId);
    void handleUpdateMilestoneStatus(const std::vector<MilestoneDTO>& milestones);
    bool handleMilestoneOption(const std::string& selectedOption, int projectId, const std::vector<MilestoneDTO>& milestones);

    ProjectClientService& projService_;
    bool keepRunning_ = true;
    void createProject();
    void viewAllProjects();
    void updateProjectDetails();
    void manageMilestones();

   protected:
    ScreenDecorator decorator() const override;
};

#endif
