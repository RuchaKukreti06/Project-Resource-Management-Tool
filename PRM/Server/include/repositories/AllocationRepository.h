#pragma once

#include "database/Database.h"
#include "repositories/IAllocationRepository.h"

class AllocationRepository : public IAllocationRepository
{
   public:
    explicit AllocationRepository(database::Database& database);

    bool createAllocation(const Allocation& allocation, int createdByUserId) override;
    bool endAllocation(int allocationId, const std::string& endDate) override;
    bool endActiveAllocationsByEmployee(int employeeId, const std::string& endDate) override;
    std::vector<Allocation> getActiveAllocationsByProject(int projectId) override;
    std::vector<Allocation> getOverlappingAllocations(int employeeId, const std::string& fromDate,
                                                      const std::string& toDate) override;
    int getOverlappingUtilization(int employeeId, const std::string& fromDate,
                                  const std::string& toDate) override;
    int getCurrentUtilization(int employeeId, const std::string& date) override;

   private:
    database::Database& database_;
};
