#pragma once

#include <memory>
#include <string>
#include <vector>

#include "models/Allocations.h"
#include "repositories/IAllocationRepository.h"
#include "repositories/IEmployeeRepository.h"
#include "repositories/IProjectRepository.h"
#include "services/interfaces/IAllocationService.h"

class AllocationService : public IAllocationService
{
   public:
    AllocationService(std::shared_ptr<IAllocationRepository> allocationRepository,
                      std::shared_ptr<IEmployeeRepository> employeeRepository,
                      std::shared_ptr<IProjectRepository> projectRepository);

    bool createAllocation(const Allocation& allocation, int createdByUserId, std::string& message) override;
    bool endAllocation(int allocationId, const std::string& endDate, std::string& message) override;
    std::vector<Allocation> getProjectAllocations(int projectId) override;
    bool recomputeEmployeeStatus(int employeeId, const std::string& todayDate) override;

   private:
    bool isProjectAllocatable(const Project& project) const;

    std::shared_ptr<IAllocationRepository> allocationRepository_;
    std::shared_ptr<IEmployeeRepository> employeeRepository_;
    std::shared_ptr<IProjectRepository> projectRepository_;
};
