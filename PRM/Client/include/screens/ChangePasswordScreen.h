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

    std::pair<std::string, std::string> promptPasswords();
    std::string promptSaveOption();
    bool validateChoice(const std::string& choice);
    bool validatePasswords(const std::string& newPassword, const std::string& confirmPassword);
    bool executePasswordChange(const std::string& newPassword);
    
    bool handleFailedChange(const std::string& message);
    void updateSessionContext();
    bool handleChangeException(const std::string& errorMessage);

   public:
    ChangePasswordScreen(AuthClientService& authService, api::ISessionStore& sessionStore);

    void show() override;
    void displayMenu() override;
    void handleInput() override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
