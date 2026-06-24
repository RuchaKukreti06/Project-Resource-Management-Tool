#ifndef ALLOCATE_RESOURCE_SCREEN_H
#define ALLOCATE_RESOURCE_SCREEN_H

#include "Screen.h"

class AiClientService;
class AllocationClientService;
class ProjectClientService;
class EmployeeClientService;

class AllocateResourceScreen : public Screen
{
   public:
    AllocateResourceScreen(AiClientService& aiService, AllocationClientService& allocService, ProjectClientService& projService, EmployeeClientService& empService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    AiClientService& aiService_;
    AllocationClientService& allocService_;
    ProjectClientService& projService_;
    EmployeeClientService& empService_;
    void findResourceAI();
    void allocateDirectly();
    void endAllocation();

   protected:
    ScreenDecorator decorator() const override;
};

#endif
