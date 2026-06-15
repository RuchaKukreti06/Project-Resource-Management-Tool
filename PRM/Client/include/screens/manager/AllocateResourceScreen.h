#ifndef ALLOCATE_RESOURCE_SCREEN_H
#define ALLOCATE_RESOURCE_SCREEN_H

#include "Screen.h"

class AllocateResourceScreen : public Screen
{
   public:
    AllocateResourceScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   private:
    void findResourceAI(ApiClient& apiClient);
    void allocateDirectly(ApiClient& apiClient);
    void endAllocation(ApiClient& apiClient);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
