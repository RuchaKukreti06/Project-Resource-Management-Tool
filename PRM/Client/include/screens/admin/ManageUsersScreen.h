#ifndef MANAGE_USERS_SCREEN_H
#define MANAGE_USERS_SCREEN_H

#include "Screen.h"

class UserClientService;
namespace api { class ISessionStore; }

class ManageUsersScreen : public Screen
{
   public:
    ManageUsersScreen(UserClientService& userService, api::ISessionStore& sessionStore);
    void show() override;
    void displayMenu() override;
    void handleInput() override;
    void createUser();
    void viewUsers();
    void resetUserPassword();
    void deactivateUser();

   private:
    UserClientService& userService_;
    api::ISessionStore& sessionStore_;
    bool keepRunning_ = true;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
