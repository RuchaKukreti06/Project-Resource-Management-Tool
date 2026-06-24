#ifndef SUBMIT_TIMESHEET_SCREEN_H
#define SUBMIT_TIMESHEET_SCREEN_H

#include "Screen.h"

class TimesheetClientService;
class AllocationClientService;
class ProjectClientService;
class EmployeeClientService;
namespace api { class ISessionStore; }

class SubmitTimesheetScreen : public Screen
{
   public:
    SubmitTimesheetScreen(TimesheetClientService& tsService, AllocationClientService& allocService, ProjectClientService& projService, EmployeeClientService& empService, api::ISessionStore& sessionStore);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    TimesheetClientService& tsService_;
    AllocationClientService& allocService_;
    ProjectClientService& projService_;
    EmployeeClientService& empService_;
    api::ISessionStore& sessionStore_;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
