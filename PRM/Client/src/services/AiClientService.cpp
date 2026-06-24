#include "services/AiClientService.h"

AiClientService::AiClientService(IApiClient& apiClient)
    : apiClient_(apiClient)
{
}

AiSkillMatchResponse AiClientService::getSkillMatch(const AiSkillMatchRequest& request)
{
    return AiSkillMatchResponse::fromJson(apiClient_.post("/ai/skill-match", request.toJson()));
}

AiRiskSummaryResponse AiClientService::getRiskSummary(const AiRiskSummaryRequest& request)
{
    return AiRiskSummaryResponse::fromJson(apiClient_.post("/ai/risk-summary", request.toJson()));
}

AiTeamBuilderResponse AiClientService::getTeamBuilder(const AiTeamBuilderRequest& request)
{
    return AiTeamBuilderResponse::fromJson(apiClient_.post("/ai/team-builder", request.toJson()));
}
