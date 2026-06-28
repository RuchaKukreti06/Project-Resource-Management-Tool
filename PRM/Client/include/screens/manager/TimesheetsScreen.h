#ifndef TIMESHEETS_SCREEN_H
#define TIMESHEETS_SCREEN_H

#include "screens/Screen.h"
#include "manager/managerConstants.h"
#include <optional>
#include <vector>
#include <string>
#include "dto/TimesheetDTO.h"

class TimesheetClientService;

class TimesheetsScreen : public Screen
{
   public:
    TimesheetsScreen(TimesheetClientService& tsService, int currentUserId);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    void displayManagerTimesheets(const std::vector<ManagerTimesheetDTO>& rows);
    void displayEmployeeTimesheets(int employeeId, const std::vector<TimesheetDTO>& timesheets);
    TimesheetClientService& tsService_;
    int currentUserId_;
    void viewTimesheetDetail();
    
    void viewTimesheetsBoard();
    std::optional<std::string> promptWeekFilter();
    void fetchAndDisplayManagerTimesheets(const std::string& formattedDate);
    
    std::optional<int> promptForEmployeeId();
    void fetchAndDisplayEmployeeTimesheets(int employeeId);

    bool keepRunning_ = true;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
