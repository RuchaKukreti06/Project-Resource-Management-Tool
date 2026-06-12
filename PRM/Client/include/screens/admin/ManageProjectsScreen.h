#ifndef MANAGE_PROJECTS_SCREEN_H
#define MANAGE_PROJECTS_SCREEN_H

#include "Screen.h"

class ManageProjectsScreen : public Screen
{
   public:
    ManageProjectsScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   private:
    bool keepRunning_ = true;
    void createProject(ApiClient& apiClient);
    void viewAllProjects(ApiClient& apiClient);
    void updateProjectDetails(ApiClient& apiClient);
    void manageMilestones(ApiClient& apiClient);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
