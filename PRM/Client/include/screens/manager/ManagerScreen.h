#ifndef MANAGER_SCREEN_H
#define MANAGER_SCREEN_H

#include "Screen.h"

#include "api/ISessionStore.h"

class Router;

class ManagerScreen : public Screen
{
   private:
    Router& router_;
    api::ISessionStore& sessionStore_;

   public:
    ManagerScreen(Router& router, api::ISessionStore& sessionStore);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
