#ifndef MANAGE_EMPLOYEES_SCREEN_H
#define MANAGE_EMPLOYEES_SCREEN_H

#include "Screen.h"

class ManageEmployeesScreen : public Screen
{
   public:
    ManageEmployeesScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   private:
    bool keepRunning_ = true;
    void viewAllEmployees(ApiClient& apiClient);
    void updateEmployee(ApiClient& apiClient);
    void deactivateEmployee(ApiClient& apiClient);
    void manageEmployeeSkills(ApiClient& apiClient);
    void assignManager(ApiClient& apiClient);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
