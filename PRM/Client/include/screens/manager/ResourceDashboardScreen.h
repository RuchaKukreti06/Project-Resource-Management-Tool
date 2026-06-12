#ifndef RESOURCE_DASHBOARD_SCREEN_H
#define RESOURCE_DASHBOARD_SCREEN_H

#include "Screen.h"

class ResourceDashboardScreen : public Screen
{
   public:
    ResourceDashboardScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   private:
    void drillIntoEmployeeDetails(ApiClient& apiClient);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
