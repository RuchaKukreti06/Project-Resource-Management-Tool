#ifndef AI_ASSISTANT_SCREEN_H
#define AI_ASSISTANT_SCREEN_H

#include "screens/Screen.h"
#include "manager/managerConstants.h"
#include "dto/AiResponseDTO.h"
#include "dto/ProjectDTO.h"
#include <vector>
#include <optional>

class AiClientService;
class ProjectClientService;

class AIAssistantScreen : public Screen
{
   public:
    AIAssistantScreen(AiClientService& aiService, ProjectClientService& projService, int currentUserId);
    void show() override;
    void displayMenu() override;
    void handleInput() override;

   private:
    AiClientService& aiService_;
    ProjectClientService& projService_;
    int currentUserId_;
    void skillMatch();
    std::string promptSkillRequirement();
    AiSkillMatchResponse generateSkillMatch(const std::string& reqText);

    void riskSummary();
    std::vector<ProjectDTO> fetchProjectsForRiskSummary();
    AiRiskSummaryResponse generateAIRiskSummary(int projectId);

    void teamBuilder();

   private:
    void displaySkillMatchResults(const std::vector<AiCandidateDTO>& candidates);
    void displayTeamMatchResults(const AiTeamBuilderResponse& aiResponse);
    void printTeamMemberRow(const AiTeamMemberDTO& item);

    bool keepRunning_ = true;
    std::optional<int> promptForProjectSelection(const std::vector<ProjectDTO>& projects);
    std::vector<std::string> wrapText(const std::string& text, int maxWidth) const;

   protected:
    ScreenDecorator decorator() const override;
};

#endif
