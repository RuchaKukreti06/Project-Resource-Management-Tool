#ifndef CHANGE_PASSWORD_SCREEN_H
#define CHANGE_PASSWORD_SCREEN_H

#include "Screen.h"

#include "services/AuthClientService.h"
#include "api/ISessionStore.h"

class ChangePasswordScreen : public Screen
{
   private:
    AuthClientService& authService_;
    api::ISessionStore& sessionStore_;

   public:
    ChangePasswordScreen(AuthClientService& authService, api::ISessionStore& sessionStore);

    void show() override;
    void displayMenu() override;
    void handleInput() override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
