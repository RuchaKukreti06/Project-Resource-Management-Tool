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

        std::vector<EmployeeDTO> activeList = fetchAndCategorizeEmployees(benchData, activeData, benchCount, activeCount);

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

std::vector<EmployeeDTO> ResourceDashboardScreen::fetchAndCategorizeEmployees(std::vector<BenchEmployeeData>& benchData, std::vector<ActiveEmployeeData>& activeData, int& benchCount, int& activeCount)
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
            std::string skillsStr = "";
            auto skillsRes = empService_.getEmployeeSkills(empId);
            if (skillsRes.success)
            {
                int count = 0;
                for (const auto& s : skillsRes.data)
                {
                    if (count > 0) skillsStr += ", ";
                    skillsStr += s.skillName;
                    count++;
                }
            }

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
}

void ResourceDashboardScreen::drillIntoEmployeeDetails()
{
    try
    {
        auto empIdOpt = promptForEmployeeId();
        if (!empIdOpt) return;
        int employeeId = empIdOpt.value();

        fetchAndDisplayEmployeeProfile(employeeId);
        displayEmployeeAllocations(employeeId);
        displayRecentActivity(employeeId);

        ConsoleInput::waitForEnter("Press Enter to go back\n");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

std::optional<int> ResourceDashboardScreen::promptForEmployeeId()
{
    std::string employeeIdInput = ConsoleInput::readLine("Enter Employee ID");
    auto parsedEmployeeId = ScreenUtils::safeParseInt(employeeIdInput);

    if (!parsedEmployeeId)
    {
        showError("Invalid employee ID format.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return std::nullopt;
    }

    return parsedEmployeeId.value();
}

void ResourceDashboardScreen::fetchAndDisplayEmployeeProfile(int employeeId)
{
    EmployeeDTO targetEmp = fetchEmployeeDTO(employeeId);
    fetchSkillsAndPrintProfile(targetEmp);
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

void ResourceDashboardScreen::fetchSkillsAndPrintProfile(const EmployeeDTO& targetEmp)
{

    std::cout << "\n── " << targetEmp.fullName << " ─────────────────────────────────\n";
    std::cout << "Department     : " << targetEmp.department << "\n";
    std::cout << "Designation    : " << targetEmp.designation << "\n";
    std::cout << "Current Status : " << targetEmp.status << "\n";

    // Fetch skills
    std::string skillsStr = "";
    auto skillsRes = empService_.getEmployeeSkills(targetEmp.id);
    if (skillsRes.success)
    {
        int count = 0;
        for (const auto& s : skillsRes.data)
        {
            if (count > 0) skillsStr += ", ";
            skillsStr += s.skillName + " (" + s.proficiency + ")";
            count++;
        }
    }
    std::cout << "Profile Skills : " << (skillsStr.empty() ? "None" : skillsStr) << "\n\n";
}

void ResourceDashboardScreen::displayEmployeeAllocations(int employeeId)
{
    std::vector<AllocationDisplayData> allocData;
    auto allocRes = allocService_.getEmployeeAllocations(employeeId);
    if (allocRes.success)
    {
        for (const auto& alloc : allocRes.data)
        {
            auto projRes = projService_.getProject(alloc.projectId);
            std::string projName =
                (projRes.success && projRes.data) ? projRes.data->name : "Unknown";

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
