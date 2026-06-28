#include "admin/AllAllocationsScreen.h"
#include "services/ProjectClientService.h"
#include "services/AllocationClientService.h"
#include "services/EmployeeClientService.h"
#include "utils/ConsoleInput.h"
#include "screens/ScreenUtils.h"
#include "api/ApiException.h"
#include <iostream>
#include <iomanip>

using namespace AdminConstants;
using namespace AdminConstants::Allocations;

AllAllocationsScreen::AllAllocationsScreen(ProjectClientService& projService,
                                           AllocationClientService& allocService,
                                           EmployeeClientService& empService)
    : projService_(projService), allocService_(allocService), empService_(empService)
{
}

ScreenDecorator AllAllocationsScreen::decorator() const
{
    return ScreenDecorator("ALL ACTIVE ALLOCATIONS").withWidth(DEFAULT_PANEL_WIDTH).withPadding(DEFAULT_PADDING);
}

void AllAllocationsScreen::displayMenu()
{
    decorator().render();
}

void AllAllocationsScreen::show()
{
    displayMenu();
    handleInput();
}

void AllAllocationsScreen::handleInput()
{
    auto rows = fetchAllocationRows();
    if (rows.has_value())
    {
        displayAllocations(rows.value());
    }
    ConsoleInput::waitForEnter("Press Enter to go back\n");
}

std::map<int, std::string> AllAllocationsScreen::buildEmployeeNameMap()
{
    std::map<int, std::string> employeeNameById;
    auto employeesResponse = empService_.viewAllEmployees();
    if (employeesResponse.success) {
        for (const auto& employee : employeesResponse.data) {
            employeeNameById[employee.id] = employee.fullName;
        }
    }
    return employeeNameById;
}

bool AllAllocationsScreen::appendProjectAllocations(
    const ProjectDTO& project, 
    const std::map<int, std::string>& employeeNameById, 
    std::vector<AllocationRow>& rows)
{
    auto allocationsResponse = allocService_.getProjectAllocations(project.id);
    if (!allocationsResponse.success)
    {
        return false;
    }
    for (const auto& alloc : allocationsResponse.data)
    {
        std::string empName = "Emp " + std::to_string(alloc.employeeId);
        auto it = employeeNameById.find(alloc.employeeId);
        if (it != employeeNameById.end()) {
            empName = it->second;
        }

        rows.push_back({
            empName,
            project.name,
            std::to_string(alloc.utilizationPercentage) + "%",
            alloc.fromDate,
            alloc.toDate
        });
    }
    return true;
}

std::optional<std::vector<AllAllocationsScreen::AllocationRow>> AllAllocationsScreen::fetchAllocationRows()
{
    try
    {
        auto response = projService_.viewAllProjects();
        if (!response.success)
        {
            showError(response.message);
            return std::nullopt;
        }
        auto projects = response.data;

        auto employeeNameById = buildEmployeeNameMap();
        std::vector<AllocationRow> rows;
        int failedProjectCount = 0;

        for (const auto& project : projects)
        {
            if (!appendProjectAllocations(project, employeeNameById, rows))
            {
                failedProjectCount++;
            }
        }
        
        if (failedProjectCount > 0)
        {
            showError("Some project allocations could not be loaded.");
        }
        
        return rows;
    }
    catch (const ApiException& e)
    {
        showError(e.what());
        return std::nullopt;
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
        return std::nullopt;
    }
}

void AllAllocationsScreen::displayAllocations(const std::vector<AllocationRow>& rows)
{
    std::cout << "\n";
    ScreenUtils::printDivider();
    std::cout << std::left 
              << std::setw(EMPLOYEE_COLUMN_WIDTH) << "Employee" 
              << std::setw(PROJECT_COLUMN_WIDTH) << "Project" 
              << std::setw(UTILIZATION_COLUMN_WIDTH) << "%" 
              << std::setw(ALLOC_DATE_COLUMN_WIDTH) << "From" 
              << std::setw(ALLOC_DATE_COLUMN_WIDTH) << "To" 
              << "\n";
    ScreenUtils::printDivider();

    for (const auto& row : rows)
    {
        std::cout << std::left
                  << std::setw(EMPLOYEE_COLUMN_WIDTH) << ScreenUtils::truncate(row.employeeName, EMPLOYEE_COLUMN_WIDTH - 2)
                  << std::setw(PROJECT_COLUMN_WIDTH) << ScreenUtils::truncate(row.projectName, PROJECT_COLUMN_WIDTH - 2)
                  << std::setw(UTILIZATION_COLUMN_WIDTH) << row.utilization
                  << std::setw(ALLOC_DATE_COLUMN_WIDTH) << ScreenUtils::valueOrDash(row.fromDate)
                  << std::setw(ALLOC_DATE_COLUMN_WIDTH) << ScreenUtils::valueOrDash(row.toDate)
                  << "\n";
    }
    ScreenUtils::printDivider();
    std::cout << "Total Active Allocations: " << rows.size() << "\n\n";
}
