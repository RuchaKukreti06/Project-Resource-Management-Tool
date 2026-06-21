#ifndef AI_ASSISTANT_SCREEN_H
#define AI_ASSISTANT_SCREEN_H

#include "Screen.h"
#include "dto/AiResponseDTO.h"

class AIAssistantScreen : public Screen
{
   public:
    AIAssistantScreen();
    void show(ApiClient& apiClient) override;
    void displayMenu() override;
    void handleInput(ApiClient& apiClient) override;

   private:
    void skillMatch(ApiClient& apiClient);
    void riskSummary(ApiClient& apiClient);
    void teamBuilder(ApiClient& apiClient);

   private:
    void displayTeamMatchResults(const AiTeamBuilderResponse& dto);

   protected:
    ScreenDecorator decorator() const override;
};

#endif
