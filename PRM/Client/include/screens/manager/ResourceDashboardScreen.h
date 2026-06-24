#ifndef RESOURCE_DASHBOARD_SCREEN_H
#define RESOURCE_DASHBOARD_SCREEN_H

#include "Screen.h"

class EmployeeClientService;
class ProjectClientService;
class AllocationClientService;
class TimesheetClientService;

class ResourceDashboardScreen : public Screen
{
   public:
    ResourceDashboardScreen(EmployeeClientService& empService, ProjectClientService& projService, AllocationClientService& allocService, TimesheetClientService& tsService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    EmployeeClientService& empService_;
    ProjectClientService& projService_;
    AllocationClientService& allocService_;
    TimesheetClientService& tsService_;
    void drillIntoEmployeeDetails();

   protected:
    ScreenDecorator decorator() const override;
};

#endif
