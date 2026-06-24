#ifndef ALLOCATION_CLIENT_SERVICE_H
#define ALLOCATION_CLIENT_SERVICE_H

#include "api/IApiClient.h"
#include "dto/AllocationDTO.h"
#include "dto/ApiResponse.h"
#include <string>

class AllocationClientService
{
private:
    IApiClient& apiClient_;

public:
    explicit AllocationClientService(IApiClient& apiClient);

    ApiEmptyResponse createAllocation(const CreateAllocationRequest& request);
    ApiEmptyResponse endAllocation(int allocationId, const std::string& endDate);
    ApiListResponse<AllocationDTO> getProjectAllocations(int projectId);
    ApiListResponse<AllocationDTO> getEmployeeAllocations(int employeeId);
};

#endif // ALLOCATION_CLIENT_SERVICE_H
