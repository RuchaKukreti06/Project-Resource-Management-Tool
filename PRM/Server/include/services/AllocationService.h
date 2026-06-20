#pragma once

#include <memory>
#include <string>
#include <vector>

#include "models/Allocations.h"
#include "repositories/IAllocationRepository.h"
#include "repositories/IEmployeeRepository.h"
#include "repositories/IProjectRepository.h"
#include "services/interfaces/IAllocationService.h"
#include "validators/AllocationValidator.h"

class AllocationService : public IAllocationService
{
   public:
    AllocationService(std::shared_ptr<IAllocationRepository> allocationRepository,
                      std::shared_ptr<IEmployeeRepository> employeeRepository,
                      std::shared_ptr<IProjectRepository> projectRepository);

    void createAllocation(const AllocationCreateRequest& req, int createdByUserId) override;
    void endAllocation(const EndAllocationRequest& req) override;
    std::vector<AllocationResponse> getProjectAllocations(int projectId) override;
    void recomputeEmployeeStatus(int employeeId, const std::string& todayDate) override;

   private:
    bool isProjectAllocatable(const Project& project) const;

    std::shared_ptr<IAllocationRepository> allocationRepository_;
    std::shared_ptr<IEmployeeRepository>   employeeRepository_;
    std::shared_ptr<IProjectRepository>    projectRepository_;
    AllocationValidator                    allocationValidator_;
};
