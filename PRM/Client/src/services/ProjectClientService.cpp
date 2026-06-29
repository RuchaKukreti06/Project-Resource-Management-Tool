#include "services/ProjectClientService.h"

ProjectClientService::ProjectClientService(IApiClient& apiClient)
    : apiClient_(apiClient)
{
}

ApiEmptyResponse ProjectClientService::createProject(const CreateProjectRequest& request)
{
    return ApiEmptyResponse::fromJson(apiClient_.post("/projects", request.toJson()));
}

ApiListResponse<ProjectDTO> ProjectClientService::viewAllProjects()
{
    return ApiListResponse<ProjectDTO>::fromJson(apiClient_.get("/projects"));
}

ApiResponse<ProjectDTO> ProjectClientService::getProject(int projectId)
{
    return ApiResponse<ProjectDTO>::fromJson(apiClient_.get("/projects/" + std::to_string(projectId)));
}

ApiEmptyResponse ProjectClientService::updateProject(int projectId, const CreateProjectRequest& request)
{
    return ApiEmptyResponse::fromJson(apiClient_.put("/projects/" + std::to_string(projectId), request.toJson()));
}

ApiListResponse<ProjectDTO> ProjectClientService::getManagerProjects(int managerId)
{
    return ApiListResponse<ProjectDTO>::fromJson(apiClient_.get("/managers/" + std::to_string(managerId) + "/projects"));
}

ApiListResponse<MilestoneDTO> ProjectClientService::getProjectMilestones(int projectId)
{
    return ApiListResponse<MilestoneDTO>::fromJson(apiClient_.get("/projects/" + std::to_string(projectId) + "/milestones"));
}

ApiEmptyResponse ProjectClientService::addMilestone(int projectId, const AddMilestoneRequest& request)
{
    return ApiEmptyResponse::fromJson(apiClient_.post("/projects/" + std::to_string(projectId) + "/milestones", request.toJson()));
}

ApiEmptyResponse ProjectClientService::updateMilestoneStatus(int milestoneId, const std::string& status)
{
    return ApiEmptyResponse::fromJson(apiClient_.put("/milestones/" + std::to_string(milestoneId) + "/status", {
        {"status", status}
    }));
}
