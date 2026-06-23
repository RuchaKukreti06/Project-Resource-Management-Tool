#include "services/AllocationService.h"
#include "dto/DTOMapper.h"
#include "exceptions/Exceptions.h"
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

void AllocationService::createAllocation(const AllocationCreateRequest& req, int createdByUserId)
{
    Allocation allocation;
    allocation.employeeId = req.employeeId;
    allocation.projectId = req.projectId;
    allocation.utilizationPercentage = req.utilizationPercentage;
    allocation.fromDate = req.fromDate;
    allocation.toDate = req.toDate;

    std::string message;
    if (!allocationValidator_.validateCreate(allocation, message))
    {
        throw exceptions::ValidationException(message);
    }

    const auto employee = employeeRepository_->getEmployeeById(allocation.employeeId);
    if (employee.id == 0 || !employee.isActive)
    {
        throw exceptions::ValidationException("Employee not active or not found.");
    }

    const auto project = projectRepository_->getProjectById(allocation.projectId);
    if (project.id == 0 || !isProjectAllocatable(project))
    {
        throw exceptions::ValidationException("Project not found or not allocatable.");
    }

    const int overlapUtilization = allocationRepository_->getOverlappingUtilization(
        allocation.employeeId, allocation.fromDate, allocation.toDate);

    if (overlapUtilization + allocation.utilizationPercentage > 100)
    {
        throw exceptions::ValidationException("Total utilization exceeds 100% in overlapping date range.");
    }

    const bool created = allocationRepository_->createAllocation(allocation, createdByUserId);
    if (!created)
    {
        throw exceptions::DatabaseException("Failed to create allocation.");
    }

    recomputeEmployeeStatus(allocation.employeeId, allocation.fromDate);
}

void AllocationService::endAllocation(const EndAllocationRequest& req)
{
    const bool ended = allocationRepository_->endAllocation(req.allocationId, req.endDate);
    if (!ended)
    {
        throw exceptions::DatabaseException("Failed to end allocation.");
    }
}

std::vector<AllocationResponse> AllocationService::getProjectAllocations(int projectId)
{
    return DTOMapper::mapToAllocationResponse(allocationRepository_->getActiveAllocationsByProject(projectId));
}

std::vector<AllocationResponse> AllocationService::getEmployeeAllocations(int employeeId)
{
    return DTOMapper::mapToAllocationResponse(allocationRepository_->getAllocationsByEmployee(employeeId));
}

void AllocationService::recomputeEmployeeStatus(int employeeId, const std::string& todayDate)
{
    const std::string effectiveDate = todayDate.empty() ? utils::currentDateIso() : todayDate;
    const int utilization = allocationRepository_->getCurrentUtilization(employeeId, effectiveDate);
    if (!employeeRepository_->setEmployeeStatus(employeeId,
                                                utilization > 0 ? "ALLOCATED" : "BENCH"))
    {
        throw exceptions::DatabaseException("Failed to set employee status.");
    }
}

std::optional<AllocationResponse> AllocationService::getAllocationById(int allocationId)
{
    auto allocation = allocationRepository_->getAllocationById(allocationId);
    if (!allocation.has_value()) {
        return std::nullopt;
    }
    return DTOMapper::mapToAllocationResponse(allocation.value());
}
