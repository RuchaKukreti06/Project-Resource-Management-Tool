#ifndef AI_CLIENT_SERVICE_H
#define AI_CLIENT_SERVICE_H

#include "api/IApiClient.h"
#include "dto/AiResponseDTO.h"

class AiClientService
{
private:
    IApiClient& apiClient_;

public:
    explicit AiClientService(IApiClient& apiClient);

    AiSkillMatchResponse getSkillMatch(const AiSkillMatchRequest& request);
    AiRiskSummaryResponse getRiskSummary(const AiRiskSummaryRequest& request);
    AiTeamBuilderResponse getTeamBuilder(const AiTeamBuilderRequest& request);
};

#endif // AI_CLIENT_SERVICE_H
