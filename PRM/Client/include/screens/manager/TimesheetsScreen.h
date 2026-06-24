#ifndef TIMESHEETS_SCREEN_H
#define TIMESHEETS_SCREEN_H

#include "Screen.h"

class TimesheetClientService;

class TimesheetsScreen : public Screen
{
   public:
    TimesheetsScreen(TimesheetClientService& tsService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    TimesheetClientService& tsService_;
    void viewTimesheetDetail();

   protected:
    ScreenDecorator decorator() const override;
};

#endif
