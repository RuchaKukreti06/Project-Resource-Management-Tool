#ifndef MY_PROJECTS_SCREEN_H
#define MY_PROJECTS_SCREEN_H

#include "Screen.h"

class ProjectClientService;
class AllocationClientService;
class EmployeeClientService;
class AiClientService;

class MyProjectsScreen : public Screen
{
   public:
    MyProjectsScreen(ProjectClientService& projService, AllocationClientService& allocService, EmployeeClientService& empService, AiClientService& aiService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    ProjectClientService& projService_;
    AllocationClientService& allocService_;
    EmployeeClientService& empService_;
    AiClientService& aiService_;
    void viewProjectDetail(int projectId);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
