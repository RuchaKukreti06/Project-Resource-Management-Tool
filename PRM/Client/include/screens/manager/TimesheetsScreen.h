#ifndef TIMESHEETS_SCREEN_H
#define TIMESHEETS_SCREEN_H

#include "Screen.h"

class TimesheetsScreen : public Screen
{
   public:
    TimesheetsScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   private:
    void viewTimesheetDetail(ApiClient& apiClient);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
