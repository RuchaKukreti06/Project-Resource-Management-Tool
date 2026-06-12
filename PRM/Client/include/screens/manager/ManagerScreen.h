#ifndef MANAGER_SCREEN_H
#define MANAGER_SCREEN_H

#include "Screen.h"

class ManagerScreen : public Screen
{
   public:
    ManagerScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
