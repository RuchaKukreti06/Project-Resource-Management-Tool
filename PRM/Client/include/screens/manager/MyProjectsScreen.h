#ifndef MY_PROJECTS_SCREEN_H
#define MY_PROJECTS_SCREEN_H

#include "screens/Screen.h"
#include <vector>
#include <string>
#include <utility>
#include <optional>
#include "dto/ProjectDTO.h"
#include "dto/AllocationDTO.h"
#include "manager/managerConstants.h"

class ProjectClientService;
class AllocationClientService;
class EmployeeClientService;
class AiClientService;

class MyProjectsScreen : public Screen
{
   public:
    MyProjectsScreen(ProjectClientService& projService, AllocationClientService& allocService, EmployeeClientService& empService, AiClientService& aiService, int currentUserId);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    ProjectClientService& projService_;
    AllocationClientService& allocService_;
    EmployeeClientService& empService_;
    AiClientService& aiService_;
    int currentUserId_;
    void viewMyProjects();
    void viewProjectDetail(int projectId);
    std::vector<std::pair<ProjectDTO, std::string>> fetchProjectsDisplayData();
    std::string calculateProjectHealth(int projectId);
    std::optional<int> promptForProjectSelection(size_t maxSelection);
    void displayMilestoneRisks(int projectId);
    void displayAllocationsDetail(int projectId);
    void handleAIRiskSummary(int projectId, const std::string& projectName);
    void displayMyProjects(const std::vector<std::pair<ProjectDTO, std::string>>& projects);
    void displayMilestones(const std::vector<MilestoneDTO>& milestones);
    void displayAllocations(const std::vector<std::pair<AllocationDTO, std::string>>& allocations);
    bool keepRunning_ = true;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
