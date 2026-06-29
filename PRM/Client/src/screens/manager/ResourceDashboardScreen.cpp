#include "manager/ResourceDashboardScreen.h"

#include <iomanip>
#include <set>
#include <sstream>

#include "api/ApiException.h"
#include "dto/AllocationDTO.h"
#include "dto/ApiResponse.h"
#include "dto/EmployeeDTO.h"
#include "dto/ProjectDTO.h"
#include "dto/TimesheetDTO.h"
#include "screens/ScreenUtils.h"
#include "services/AllocationClientService.h"
#include "services/EmployeeClientService.h"
#include "services/ProjectClientService.h"
#include "services/TimesheetClientService.h"
#include "utils/ConsoleInput.h"

using namespace ManagerConstants;
using namespace ManagerConstants::ResourceDashboard;

ResourceDashboardScreen::ResourceDashboardScreen(EmployeeClientService& empService,
                                                 ProjectClientService& projService,
                                                 AllocationClientService& allocService,
                                                 TimesheetClientService& tsService,
                                                 int currentUserId)
    : empService_(empService),
      projService_(projService),
      allocService_(allocService),
      tsService_(tsService),
      currentUserId_(currentUserId)
{
}

void ResourceDashboardScreen::displayMenu()
{
}

void ResourceDashboardScreen::show()
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        viewDashboard();
        if (keepRunning_)
        {
            handleInput();
        }
    }
}

void ResourceDashboardScreen::viewDashboard()
{
    try
    {
        std::vector<BenchEmployeeData> benchData;
        std::vector<ActiveEmployeeData> activeData;
        int benchCount = 0;
        int activeCount = 0;
        std::vector<EmployeeDTO> activeList = categorizeEmployees(benchData, activeData, benchCount, activeCount);

        decorator().render();
        displayBenchEmployees(benchData);

        for (const auto& emp : activeList)
        {
            int util = emp.totalUtilisation;
            std::string avail =
                (util == 0) ? "FULL"
                            : (util >= 100 ? "0% free" : std::to_string(100 - util) + "% free");

            activeData.push_back({emp.id, emp.fullName, std::to_string(util) + "%", avail});
        }
        displayActiveEmployees(activeData);
        printDashboardSummary(benchCount, activeCount);
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        keepRunning_ = false;
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
        keepRunning_ = false;
    }
}

std::vector<EmployeeDTO> ResourceDashboardScreen::categorizeEmployees(std::vector<BenchEmployeeData>& benchData, std::vector<ActiveEmployeeData>& activeData, int& benchCount, int& activeCount)
{
    auto response = empService_.getTeamEmployees(currentUserId_);
    if (!response.success)
    {
        throw ApiException(response.message);
    }

    auto employees = response.data;
    std::vector<EmployeeDTO> activeList;

    for (const auto& emp : employees)
    {
        if (!emp.isActive) continue;

        int empId = emp.id;
        std::string status = emp.status;

        if (status == "BENCH")
        {
            std::string skillsStr = buildSkillSummary(empId, false);
            benchData.push_back({empId, emp.fullName, emp.department, skillsStr});
            benchCount++;
        }
        else
        {
            activeList.push_back(emp);
            activeCount++;
        }
    }
    return activeList;
}

void ResourceDashboardScreen::printDashboardSummary(int benchCount, int activeCount)
{
    std::cout << "Bench: " << benchCount << "   |   Active: " << activeCount << "\n\n";
}

void ResourceDashboardScreen::handleInput()
{
    std::cout << "[" << OPT_DRILL_DOWN << "] Drill into employee details     [" << OPT_BACK
              << "] Back\n";
    std::string choice = ConsoleInput::readLine("Enter choice");
    if (ScreenUtils::equalsIgnoreCase(choice, OPT_DRILL_DOWN))
    {
        drillIntoEmployeeDetails();
    }
    else if (ScreenUtils::equalsIgnoreCase(choice, OPT_BACK))
    {
        keepRunning_ = false;
    }
    else
    {
        showError(Messages::INVALID_OPTION);
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
    }
}

void ResourceDashboardScreen::drillIntoEmployeeDetails()
{
    try
    {
        auto empIdOpt = promptForEmployeeId();
        if (!empIdOpt) return;
        int employeeId = empIdOpt.value();

        fetchEmployeeProfile(employeeId);
        displayEmployeeAllocations(employeeId);
        displayRecentActivity(employeeId);

        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_GO_BACK);
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
    }
    catch (const std::exception&)
    {
        showError(Messages::SOMETHING_WENT_WRONG);
    }
}

std::optional<int> ResourceDashboardScreen::promptForEmployeeId()
{
    std::string employeeIdInput = ConsoleInput::readLine("Enter Employee ID");
    auto parsedEmployeeId = ScreenUtils::safeParseInt(employeeIdInput);

    if (!parsedEmployeeId)
    {
        showError(Messages::INVALID_SELECTION_FORMAT);
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
        return std::nullopt;
    }

    return parsedEmployeeId.value();
}

void ResourceDashboardScreen::fetchEmployeeProfile(int employeeId)
{
    EmployeeDTO targetEmployee = fetchEmployeeDTO(employeeId);
    printEmployeeProfile(targetEmployee);
}

EmployeeDTO ResourceDashboardScreen::fetchEmployeeDTO(int employeeId)
{
    auto response = empService_.getTeamEmployees(currentUserId_);
    if (!response.success) throw ApiException(response.message);

    for (const auto& emp : response.data)
    {
        if (emp.id == employeeId)
        {
            return emp;
        }
    }
    throw ApiException("Employee not found in your team.");
}

void ResourceDashboardScreen::printEmployeeProfile(const EmployeeDTO& targetEmployee)
{
    std::cout << "\n── " << targetEmployee.fullName << " ─────────────────────────────────\n";
    std::cout << "Department     : " << targetEmployee.department << "\n";
    std::cout << "Designation    : " << targetEmployee.designation << "\n";
    std::cout << "Current Status : " << targetEmployee.status << "\n";

    std::string skillsStr = buildSkillSummary(targetEmployee.id, true);
    std::cout << "Profile Skills : " << (skillsStr.empty() ? "None" : skillsStr) << "\n\n";
}

std::string ResourceDashboardScreen::buildSkillSummary(int employeeId, bool includeProficiency)
{
    std::string skillsSummary;
    auto skillsResponse = empService_.getEmployeeSkills(employeeId);
    if (!skillsResponse.success)
    {
        return skillsSummary;
    }
    int count = 0;
    for (const auto& skill : skillsResponse.data)
    {
        if (count > 0)
        {
            skillsSummary += ", ";
        }
        skillsSummary += skill.skillName;
        if (includeProficiency)
        {
            skillsSummary += " (" + skill.proficiency + ")";
        }
        ++count;
    }
    return skillsSummary;
}

void ResourceDashboardScreen::displayEmployeeAllocations(int employeeId)
{
    std::vector<AllocationDisplayData> allocData;
    auto allocationResponse = allocService_.getEmployeeAllocations(employeeId);
    if (allocationResponse.success)
    {
        for (const auto& alloc : allocationResponse.data)
        {
            auto projectResponse = projService_.getProject(alloc.projectId);
            std::string projName =
                (projectResponse.success && projectResponse.data) ? projectResponse.data->name : "Unknown";

            allocData.push_back({projName, std::to_string(alloc.utilizationPercentage) + "%",
                                 alloc.fromDate, alloc.toDate});
        }
    }

    displayAllocations(allocData);
    std::cout << "\n";
}

void ResourceDashboardScreen::displayRecentActivity(int employeeId)
{
    std::cout << "Recent Activity Tags (last 4 weeks):\n  ";
    auto tsRes = tsService_.getEmployeeTimesheets(employeeId);
    std::set<std::string> tagSet;
    if (tsRes.success)
    {
        int weekCount = 0;
        for (const auto& ts : tsRes.data)
        {
            if (weekCount++ >= 4) break;
            auto detailRes = tsService_.getTimesheetEntries(ts.id);
            if (detailRes.success)
            {
                for (const auto& row : detailRes.data)
                {
                    std::string tags = row.tags;
                    std::istringstream ss(tags);
                    std::string tag;
                    while (std::getline(ss, tag, ','))
                    {
                        auto start = tag.find_first_not_of(" ");
                        if (start != std::string::npos) tagSet.insert(tag.substr(start));
                    }
                }
            }
        }
    }
    std::string tagsStr;
    for (const auto& t : tagSet)
    {
        if (!tagsStr.empty()) tagsStr += ", ";
        tagsStr += t;
    }
    std::cout << (tagsStr.empty() ? "No recent activity recorded." : tagsStr) << "\n\n";
}

ScreenDecorator ResourceDashboardScreen::decorator() const
{
    return ScreenDecorator("RESOURCE DASHBOARD")
        .withWidth(ManagerConstants::DEFAULT_PANEL_WIDTH)
        .withPadding(ManagerConstants::DEFAULT_PADDING);
}

void ResourceDashboardScreen::displayBenchEmployees(
    const std::vector<BenchEmployeeData>& benchEmployees)
{
    std::cout << "\n================ ON BENCH ================\n";
    std::cout << std::left << std::setw(6) << "ID" << std::setw(20) << "Name" << std::setw(15)
              << "Department" << std::setw(25) << "Skills" << "\n";
    std::cout << "──────────────────────────────────────────────\n";

    for (const auto& emp : benchEmployees)
    {
        std::cout << std::left << std::setw(6) << emp.id << std::setw(20)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(emp.name), 19) << std::setw(15)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(emp.department), 14)
                  << std::setw(25)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(emp.skills), 24) << "\n";
    }
    std::cout << "──────────────────────────────────────────────\n";
}

void ResourceDashboardScreen::displayActiveEmployees(
    const std::vector<ActiveEmployeeData>& activeEmployees)
{
    std::cout << "\n================ ACTIVE EMPLOYEES ================\n";
    std::cout << std::left << std::setw(6) << "ID" << std::setw(20) << "Name" << std::setw(15)
              << "Alloc %" << std::setw(15) << "Availability" << "\n";
    std::cout << "────────────────────────────────────────────────\n";

    for (const auto& emp : activeEmployees)
    {
        std::cout << std::left << std::setw(6) << emp.id << std::setw(20)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(emp.name), 19) << std::setw(15)
                  << ScreenUtils::valueOrDash(emp.allocPercent) << std::setw(15)
                  << ScreenUtils::valueOrDash(emp.availability) << "\n";
    }
    std::cout << "────────────────────────────────────────────────\n";
}

void ResourceDashboardScreen::displayAllocations(
    const std::vector<AllocationDisplayData>& allocations)
{
    std::cout << "\n================ Active Allocations ================\n";
    std::cout << std::left << std::setw(20) << "Project" << std::setw(8) << "%" << std::setw(12)
              << "From" << std::setw(12) << "To" << "\n";
    std::cout << "────────────────────────────────────────────────────\n";

    for (const auto& alloc : allocations)
    {
        std::cout << std::left << std::setw(20)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(alloc.projectName), 19)
                  << std::setw(8) << ScreenUtils::valueOrDash(alloc.utilPercent) << std::setw(12)
                  << ScreenUtils::valueOrDash(alloc.fromDate) << std::setw(12)
                  << ScreenUtils::valueOrDash(alloc.toDate) << "\n";
    }
    std::cout << "────────────────────────────────────────────────────\n";
}
