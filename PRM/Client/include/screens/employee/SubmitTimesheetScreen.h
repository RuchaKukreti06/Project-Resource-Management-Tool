#ifndef SUBMIT_TIMESHEET_SCREEN_H
#define SUBMIT_TIMESHEET_SCREEN_H

#include "Screen.h"

class SubmitTimesheetScreen : public Screen
{
   public:
    SubmitTimesheetScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
