#ifndef MANAGE_PROJECTS_SCREEN_H
#define MANAGE_PROJECTS_SCREEN_H

#include "Screen.h"

class ProjectClientService;

class ManageProjectsScreen : public Screen
{
   public:
    ManageProjectsScreen(ProjectClientService& projService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
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
