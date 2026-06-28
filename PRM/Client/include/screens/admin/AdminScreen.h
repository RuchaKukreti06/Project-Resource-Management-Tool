#ifndef ADMIN_SCREEN_H
#define ADMIN_SCREEN_H

#include "screens/Screen.h"
#include "admin/adminConstants.h"

#include "api/ISessionStore.h"

class Router;

class AdminScreen : public Screen
{
   private:
    Router& router_;
    api::ISessionStore& sessionStore_;

   public:
    AdminScreen(Router& router, api::ISessionStore& sessionStore);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
