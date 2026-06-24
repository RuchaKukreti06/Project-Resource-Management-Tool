#ifndef PROJECT_CLIENT_SERVICE_H
#define PROJECT_CLIENT_SERVICE_H

#include "api/IApiClient.h"
#include "dto/ProjectDTO.h"
#include "dto/ApiResponse.h"
#include <string>

class ProjectClientService
{
private:
    IApiClient& apiClient_;

public:
    explicit ProjectClientService(IApiClient& apiClient);

    ApiEmptyResponse createProject(const CreateProjectRequest& request);
    ApiListResponse<ProjectDTO> viewAllProjects();
    ApiResponse<ProjectDTO> getProject(int projectId);
    ApiEmptyResponse updateProject(int projectId, const CreateProjectRequest& request);
    ApiListResponse<ProjectDTO> getManagerProjects(int managerId);

    ApiListResponse<MilestoneDTO> getProjectMilestones(int projectId);
    ApiEmptyResponse addMilestone(int projectId, const AddMilestoneRequest& request);
    ApiEmptyResponse updateMilestoneStatus(int milestoneId, const std::string& status);
};

#endif // PROJECT_CLIENT_SERVICE_H
