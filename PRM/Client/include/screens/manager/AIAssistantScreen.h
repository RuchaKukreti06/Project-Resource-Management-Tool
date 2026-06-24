#ifndef AI_ASSISTANT_SCREEN_H
#define AI_ASSISTANT_SCREEN_H

#include "Screen.h"
#include "dto/AiResponseDTO.h"

class AiClientService;
class ProjectClientService;

class AIAssistantScreen : public Screen
{
   public:
    AIAssistantScreen(AiClientService& aiService, ProjectClientService& projService);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    AiClientService& aiService_;
    ProjectClientService& projService_;
    void skillMatch();
    void riskSummary();
    void teamBuilder();

   private:
    void displayTeamMatchResults(const AiTeamBuilderResponse& dto);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
