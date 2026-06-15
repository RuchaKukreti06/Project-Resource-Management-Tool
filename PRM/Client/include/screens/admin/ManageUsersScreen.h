#ifndef MANAGE_USERS_SCREEN_H
#define MANAGE_USERS_SCREEN_H

#include "Screen.h"

class ManageUsersScreen : public Screen
{
   public:
    ManageUsersScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;
    void createUser(ApiClient& apiClient);
    void viewUsers(ApiClient& apiClient);
    void resetUserPassword(ApiClient& apiClient);
    void deactivateUser(ApiClient& apiClient);

   private:
    bool keepRunning_ = true;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
