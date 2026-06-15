#ifndef ADMIN_SCREEN_H
#define ADMIN_SCREEN_H

#include "Screen.h"

class AdminScreen : public Screen
{
   public:
    AdminScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
