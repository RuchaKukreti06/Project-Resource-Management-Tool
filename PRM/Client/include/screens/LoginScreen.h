#ifndef LOGIN_SCREEN_H
#define LOGIN_SCREEN_H

#include "Screen.h"

#include "services/AuthClientService.h"
#include "api/ISessionStore.h"

class Router;

class LoginScreen : public Screen
{
   private:
    Router& router_;
    AuthClientService& authService_;
    api::ISessionStore& sessionStore_;
    IApiClient& apiClient_;

   public:
    LoginScreen(Router& router, AuthClientService& authService, api::ISessionStore& sessionStore, IApiClient& apiClient);

    void show() override;
    void displayMenu() override;
    void handleInput() override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
