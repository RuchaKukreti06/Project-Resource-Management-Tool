#ifndef CHANGE_PASSWORD_SCREEN_H
#define CHANGE_PASSWORD_SCREEN_H

#include "Screen.h"

class ChangePasswordScreen : public Screen
{
   public:
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
