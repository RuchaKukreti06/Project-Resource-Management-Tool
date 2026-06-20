#include "services/AllocationService.h"
#include "dto/DTOMapper.h"

#include "utils/DateUtils.h"

AllocationService::AllocationService(std::shared_ptr<IAllocationRepository> allocationRepository,
                                     std::shared_ptr<IEmployeeRepository> employeeRepository,
                                     std::shared_ptr<IProjectRepository> projectRepository)
    : allocationRepository_(std::move(allocationRepository)),
      employeeRepository_(std::move(employeeRepository)),
      projectRepository_(std::move(projectRepository))
{
}

bool AllocationService::isProjectAllocatable(const Project& project) const
{
    return project.status == "ACTIVE" || project.status == "PLANNED";
}

bool AllocationService::createAllocation(const AllocationCreateRequest& req, int createdByUserId,
                                         std::string& message)
{
    Allocation allocation;
    allocation.employeeId = req.employeeId;
    allocation.projectId = req.projectId;
    allocation.utilizationPercentage = req.utilizationPercentage;
    allocation.fromDate = req.fromDate;
    allocation.toDate = req.toDate;

    if (!allocationValidator_.validateCreate(allocation, message))
    {
        return false;
    }

    const auto employee = employeeRepository_->getEmployeeById(allocation.employeeId);
    if (employee.id == 0 || !employee.isActive)
    {
        message = "Employee not active or not found.";
        return false;
    }

    const auto project = projectRepository_->getProjectById(allocation.projectId);
    if (project.id == 0 || !isProjectAllocatable(project))
    {
        message = "Project not found or not allocatable.";
        return false;
    }

    const int overlapUtilization = allocationRepository_->getOverlappingUtilization(
        allocation.employeeId, allocation.fromDate, allocation.toDate);

    if (overlapUtilization + allocation.utilizationPercentage > 100)
    {
        message = "Total utilization exceeds 100% in overlapping date range.";
        return false;
    }

    const bool created = allocationRepository_->createAllocation(allocation, createdByUserId);
    if (!created)
    {
        message = "Failed to create allocation.";
        return false;
    }

    recomputeEmployeeStatus(allocation.employeeId, allocation.fromDate);
    message = "Allocation created.";
    return true;
}

bool AllocationService::endAllocation(const EndAllocationRequest& req, std::string& message)
{
    const bool ended = allocationRepository_->endAllocation(req.allocationId, req.endDate);
    message = ended ? "Allocation ended." : "Failed to end allocation.";
    return ended;
}

std::vector<AllocationResponse> AllocationService::getProjectAllocations(int projectId)
{
    return DTOMapper::mapToAllocationResponse(allocationRepository_->getActiveAllocationsByProject(projectId));
}

bool AllocationService::recomputeEmployeeStatus(int employeeId, const std::string& todayDate)
{
    const std::string effectiveDate = todayDate.empty() ? utils::currentDateIso() : todayDate;
    const int utilization = allocationRepository_->getCurrentUtilization(employeeId, effectiveDate);
    return employeeRepository_->setEmployeeStatus(employeeId,
                                                  utilization > 0 ? "ALLOCATED" : "BENCH");
}
