#ifndef EMPLOYEE_SCREEN_H
#define EMPLOYEE_SCREEN_H

#include "Screen.h"

class Router;
namespace api { class ISessionStore; }
class EmployeeClientService;
class TimesheetClientService;
class ProjectClientService;
class AllocationClientService;
struct TimesheetEntryDTO;

class EmployeeScreen : public Screen
{
   public:
    EmployeeScreen(Router& router, api::ISessionStore& sessionStore, EmployeeClientService& empService, TimesheetClientService& tsService, ProjectClientService& projService, AllocationClientService& allocService);
    void show() override;

   private:
    struct TimesheetDisplayData {
        std::string dispDate;
        std::string totalHrs;
        std::string status;
    };
    
    struct AllocationDisplayData {
        std::string projectName;
        std::string utilPercent;
        std::string fromDate;
        std::string toDate;
        std::string status;
    };

    void displayMyTimesheets(const std::vector<TimesheetDisplayData>& timesheets);
    void displayMyAllocations(const std::vector<AllocationDisplayData>& allocations, int totalUtil);
    void displayTimesheetDetailsList(const std::string& title, const std::vector<TimesheetEntryDTO>& entries, int totalHrs);

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
