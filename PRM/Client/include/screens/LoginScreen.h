#ifndef LOGIN_SCREEN_H
#define LOGIN_SCREEN_H

#include "Screen.h"

class LoginScreen : public Screen
{
   public:
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
