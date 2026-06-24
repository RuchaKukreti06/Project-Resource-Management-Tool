#ifndef EMPLOYEE_SCREEN_H
#define EMPLOYEE_SCREEN_H

#include "Screen.h"

class Router;
namespace api { class ISessionStore; }
class EmployeeClientService;
class TimesheetClientService;
class ProjectClientService;
class AllocationClientService;

class EmployeeScreen : public Screen
{
   public:
    EmployeeScreen(Router& router, api::ISessionStore& sessionStore, EmployeeClientService& empService, TimesheetClientService& tsService, ProjectClientService& projService, AllocationClientService& allocService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    Router& router_;
    api::ISessionStore& sessionStore_;
    EmployeeClientService& empService_;
    TimesheetClientService& tsService_;
    ProjectClientService& projService_;
    AllocationClientService& allocService_;

    void viewMyTimesheets();
    void viewMyAllocations();
    void viewTimesheetDetails(int timesheetId, const std::string& weekStart, const std::string& status);

    bool hasMissingTimesheet_ = false;
    std::string missingWeekStr_ = "";

   protected:
    ScreenDecorator decorator() const override;
};

#endif
