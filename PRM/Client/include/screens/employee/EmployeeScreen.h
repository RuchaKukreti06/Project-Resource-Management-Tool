#ifndef EMPLOYEE_SCREEN_H
#define EMPLOYEE_SCREEN_H

#include "Screen.h"

class EmployeeScreen : public Screen
{
   public:
    EmployeeScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   private:
    void viewMyTimesheets(ApiClient& apiClient);
    void viewMyAllocations(ApiClient& apiClient);
    void viewTimesheetDetails(ApiClient& apiClient, int timesheetId, const std::string& weekStart, const std::string& status);

    bool hasMissingTimesheet_ = false;
    std::string missingWeekStr_ = "";

   protected:
    ScreenDecorator decorator() const override;
};

#endif
