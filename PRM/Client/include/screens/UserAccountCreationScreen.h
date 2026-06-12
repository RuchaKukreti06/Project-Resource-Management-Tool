#ifndef USER_ACCOUNT_CREATION_SCREEN_H
#define USER_ACCOUNT_CREATION_SCREEN_H

#include "Screen.h"

class UserAccountCreationScreen : public Screen
{
   public:
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif