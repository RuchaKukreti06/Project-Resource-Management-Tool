#ifndef TIMESHEETS_SCREEN_H
#define TIMESHEETS_SCREEN_H

#include "screens/Screen.h"
#include "manager/managerConstants.h"
#include <optional>
#include <vector>
#include <string>
#include "dto/TimesheetDTO.h"

class TimesheetClientService;
class EmployeeClientService;

class TimesheetsScreen : public Screen
{
   public:
    TimesheetsScreen(TimesheetClientService& tsService, EmployeeClientService& empService, int currentUserId);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    void displayManagerTimesheets(const std::vector<ManagerTimesheetDTO>& rows);
    void displayEmployeeTimesheets(int employeeId, const std::vector<TimesheetDTO>& timesheets);
    TimesheetClientService& tsService_;
    EmployeeClientService& empService_;
    int currentUserId_;
    void viewTimesheetDetail();
    
    void viewTimesheetsBoard();
    std::optional<std::string> promptWeekFilter();
    void fetchManagerTimesheets(const std::string& formattedDate);
    
    std::optional<int> promptForEmployeeId();
    void fetchEmployeeTimesheets(int employeeId);

    bool keepRunning_ = true;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
