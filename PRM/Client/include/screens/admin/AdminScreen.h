#ifndef ADMIN_SCREEN_H
#define ADMIN_SCREEN_H

#include "Screen.h"

#include "api/ISessionStore.h"

class Router;

class ProjectClientService;
class AllocationClientService;
class EmployeeClientService;

class AdminScreen : public Screen
{
   private:
    Router& router_;
    api::ISessionStore& sessionStore_;
    ProjectClientService& projService_;
    AllocationClientService& allocService_;
    EmployeeClientService& empService_;

   public:
    AdminScreen(Router& router, api::ISessionStore& sessionStore, ProjectClientService& projService, AllocationClientService& allocService, EmployeeClientService& empService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
