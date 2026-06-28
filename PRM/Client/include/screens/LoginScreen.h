#ifndef LOGIN_SCREEN_H
#define LOGIN_SCREEN_H

#include "Screen.h"

#include "services/AuthClientService.h"
#include "api/ISessionStore.h"
#include "dto/AuthDTO.h"

class Router;

class LoginScreen : public Screen
{
   private:
    Router& router_;
    AuthClientService& authService_;
    api::ISessionStore& sessionStore_;
    IApiClient& apiClient_;

    bool processLoginOption();
    std::pair<std::string, std::string> promptCredentials();
    bool validateCredentials(const std::string& username, const std::string& password);
    bool executeLogin(const std::string& username, const std::string& password);
    
    bool handleFailedLogin(const std::string& message);
    void updateSessionContext(const AuthLoginResponse& loginRes);
    void displayLoginResult(bool forcePasswordChange);
    bool handleLoginException(const std::string& errorMessage);
    
    void handleExitOption();

   public:
    LoginScreen(Router& router, AuthClientService& authService, api::ISessionStore& sessionStore, IApiClient& apiClient);

    void show() override;
    void displayMenu() override;
    void handleInput() override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
