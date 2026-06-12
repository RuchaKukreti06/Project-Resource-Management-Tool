#ifndef MY_PROJECTS_SCREEN_H
#define MY_PROJECTS_SCREEN_H

#include "Screen.h"

class MyProjectsScreen : public Screen
{
   public:
    MyProjectsScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   private:
    void viewProjectDetail(ApiClient& apiClient, int projectId);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
