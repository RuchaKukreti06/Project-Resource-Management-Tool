#include "services/AllocationClientService.h"

AllocationClientService::AllocationClientService(IApiClient& apiClient)
    : apiClient_(apiClient)
{
}

ApiEmptyResponse AllocationClientService::createAllocation(const CreateAllocationRequest& request)
{
    return ApiEmptyResponse::fromJson(apiClient_.post("/allocations", request.toJson()));
}

ApiEmptyResponse AllocationClientService::endAllocation(int allocationId, const std::string& endDate)
{
    return ApiEmptyResponse::fromJson(apiClient_.put("/allocations/" + std::to_string(allocationId) + "/end", {
        {"end_date", endDate}
    }));
}

ApiListResponse<AllocationDTO> AllocationClientService::getProjectAllocations(int projectId)
{
    return ApiListResponse<AllocationDTO>::fromJson(apiClient_.get("/projects/" + std::to_string(projectId) + "/allocations"));
}

ApiListResponse<AllocationDTO> AllocationClientService::getEmployeeAllocations(int employeeId)
{
    return ApiListResponse<AllocationDTO>::fromJson(apiClient_.get("/employees/" + std::to_string(employeeId) + "/allocations"));
}
