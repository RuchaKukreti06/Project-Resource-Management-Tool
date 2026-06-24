#ifndef MANAGE_EMPLOYEES_SCREEN_H
#define MANAGE_EMPLOYEES_SCREEN_H

#include "Screen.h"

class EmployeeClientService;

class ManageEmployeesScreen : public Screen
{
   public:
    ManageEmployeesScreen(EmployeeClientService& empService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    EmployeeClientService& empService_;
    bool keepRunning_ = true;
    void viewAllEmployees();
    void updateEmployee();
    void deactivateEmployee();
    void manageEmployeeSkills();
    void assignManager();

   protected:
    ScreenDecorator decorator() const override;
};

#endif
