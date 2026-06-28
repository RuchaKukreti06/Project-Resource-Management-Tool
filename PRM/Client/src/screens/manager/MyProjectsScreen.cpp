#include "manager/MyProjectsScreen.h"

#include <map>
#include <iomanip>
#include "api/ApiException.h"
#include "dto/AiResponseDTO.h"
#include "dto/AllocationDTO.h"
#include "dto/ApiResponse.h"
#include "dto/EmployeeDTO.h"
#include "dto/ProjectDTO.h"
#include "screens/ScreenUtils.h"
#include "services/AiClientService.h"
#include "services/AllocationClientService.h"
#include "services/EmployeeClientService.h"
#include "services/ProjectClientService.h"
#include "utils/ConsoleInput.h"
#include "utils/DateUtils.h"

using namespace ManagerConstants;
using namespace ManagerConstants::MyProjects;

MyProjectsScreen::MyProjectsScreen(ProjectClientService& projService,
                                   AllocationClientService& allocService,
                                   EmployeeClientService& empService, AiClientService& aiService,
                                   int currentUserId)
    : projService_(projService),
      allocService_(allocService),
      empService_(empService),
      aiService_(aiService),
      currentUserId_(currentUserId)
{
}

void MyProjectsScreen::displayMenu()
{
}

void MyProjectsScreen::show()
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        viewMyProjects();
    }
}

void MyProjectsScreen::viewMyProjects()
{
    try
    {
        auto displayData = fetchProjectsDisplayData();
        decorator().render();
        displayMyProjects(displayData);

        if (displayData.empty())
        {
            showInfo("You have no assigned projects.");
            ConsoleInput::waitForEnter("Press Enter to go back");
            keepRunning_ = false;
            return;
        }

        auto selectionOpt = promptForProjectSelection(displayData.size());
        if (!selectionOpt)
        {
            keepRunning_ = false;
            return;
        }

        viewProjectDetail(displayData[selectionOpt.value() - 1].first.id);
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue");
        keepRunning_ = false;
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
        keepRunning_ = false;
    }
}

std::vector<std::pair<ProjectDTO, std::string>> MyProjectsScreen::fetchProjectsDisplayData()
{
    int managerId = currentUserId_;
    auto response = projService_.getManagerProjects(managerId);

    if (!response.success)
    {
        throw ApiException(response.message);
    }

    std::vector<std::pair<ProjectDTO, std::string>> displayData;
    for (const auto& proj : response.data)
    {
        std::string health = calculateProjectHealth(proj.id);
        displayData.push_back({proj, health});
    }
    return displayData;
}

std::string MyProjectsScreen::calculateProjectHealth(int projectId)
{
    auto msRes = projService_.getProjectMilestones(projectId);

    std::string health = "🟢 ON TRACK";
    bool hasOverdue = false;
    bool hasApproaching = false;

    std::string todayStr = DateUtils::getCurrentDateYYYYMMDD();
    std::string nextStr = DateUtils::getDateNextWeekYYYYMMDD();

    if (msRes.success)
    {
        for (const auto& ms : msRes.data)
        {
            std::string status = ms.status;
            std::string dueDate = ms.dueDate;
            if (status != "DONE" && !dueDate.empty() && dueDate < todayStr)
            {
                hasOverdue = true;
            }
            else if (status != "DONE" && !dueDate.empty() && dueDate <= nextStr)
            {
                hasApproaching = true;
            }
        }
    }

    if (hasOverdue)
    {
        health = "🔴 AT RISK";
    }
    else if (hasApproaching)
    {
        health = "🟡 ATTENTION";
    }
    return health;
}

std::optional<int> MyProjectsScreen::promptForProjectSelection(size_t maxSelection)
{
    std::string selStr =
        ConsoleInput::readLine("Select project number to view details (or 0 to go back)");
    auto parsedSel = ScreenUtils::safeParseInt(selStr);
    if (!parsedSel)
    {
        showError("Invalid selection format.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return std::nullopt;
    }
    int selection = parsedSel.value();
    if (selection == 0 || selection < 1 || selection > (int)maxSelection)
    {
        return std::nullopt;
    }
    return selection;
}

void MyProjectsScreen::handleInput()
{
}

void MyProjectsScreen::viewProjectDetail(int projectId)
{
    while (true)
    {
        try
        {
            auto projRes = projService_.getProject(projectId);
            if (!projRes.success || !projRes.data) return;
            auto proj = *projRes.data;

            std::cout << "\n── " << proj.name << " ───────────────────────────────\n";

            displayMilestoneRisks(projectId);
            displayAllocationsDetail(projectId);

            std::cout << "[" << OPT_AI_RISK_SUMMARY << "] Get AI Risk Summary     [" << OPT_BACK
                      << "] Back\n";
            std::string choice = ConsoleInput::readLine("Enter option");
            if (ScreenUtils::equalsIgnoreCase(choice, OPT_AI_RISK_SUMMARY))
            {
                handleAIRiskSummary(projectId, proj.name);
            }
            else if (ScreenUtils::equalsIgnoreCase(choice, OPT_BACK))
            {
                break;
            }
        }
        catch (const ApiException& ex)
        {
            showError(ex.what());
            ConsoleInput::waitForEnter("Press Enter to continue");
            break;
        }
        catch (const std::exception&)
        {
            showError("Something went wrong. Please try again.");
            break;
        }
    }
}

void MyProjectsScreen::displayMilestoneRisks(int projectId)
{
    auto msRes = projService_.getProjectMilestones(projectId);

    std::string todayStr = DateUtils::getCurrentDateYYYYMMDD();
    std::string nextStr = DateUtils::getDateNextWeekYYYYMMDD();

    bool hasOverdue = false;
    bool hasApproaching = false;
    std::vector<std::string> overdueTitles;
    if (msRes.success)
    {
        for (const auto& ms : msRes.data)
        {
            if (ms.status != "DONE" && !ms.dueDate.empty() && ms.dueDate < todayStr)
            {
                hasOverdue = true;
                overdueTitles.push_back(ms.title);
            }
            else if (ms.status != "DONE" && !ms.dueDate.empty() && ms.dueDate <= nextStr)
            {
                hasApproaching = true;
            }
        }
    }

    std::string healthStatus = "🟢 ON TRACK";
    if (hasOverdue)
        healthStatus = "🔴 AT RISK";
    else if (hasApproaching)
        healthStatus = "🟡 ATTENTION";

    std::cout << "Health Status : " << healthStatus << "\n\n";
    if (hasOverdue)
    {
        std::cout << "Risk Flags:\n";
        for (const auto& title : overdueTitles)
        {
            std::cout << "  ✗ " << title << " milestone is overdue\n";
        }
        std::cout << "\n";
    }

    std::vector<MilestoneDTO> milestones = msRes.success ? msRes.data : std::vector<MilestoneDTO>{};
    displayMilestones(milestones);
}

void MyProjectsScreen::displayAllocationsDetail(int projectId)
{
    auto allocsRes = allocService_.getProjectAllocations(projectId);

    std::vector<std::pair<AllocationDTO, std::string>> allocData;
    std::map<int, std::string> employeeNameById;
    auto empsRes = empService_.getTeamEmployees(currentUserId_); // Safe API call
    if (empsRes.success)
    {
        for (const auto& e : empsRes.data)
        {
            employeeNameById[e.id] = e.fullName;
        }
    }

    for (const auto& alloc : (allocsRes.success ? allocsRes.data : std::vector<AllocationDTO>{}))
    {
        int empId = alloc.employeeId;
        std::string empName = "Emp " + std::to_string(empId);
        auto it = employeeNameById.find(empId);
        if (it != employeeNameById.end())
        {
            empName = it->second;
        }
        allocData.push_back({alloc, empName});
    }
    displayAllocations(allocData);
}

void MyProjectsScreen::handleAIRiskSummary(int projectId, const std::string& projectName)
{
    std::cout << "\nGenerating AI summary...\n\n";
    AiRiskSummaryRequest req;
    req.projectId = projectId;
    auto aiRes = aiService_.getRiskSummary(req);

    std::cout << "\n── AI Risk Summary — " << projectName << " ────────────\n\n";

    if (!aiRes.fallback_message.has_value())
    {
        std::cout << "\"" << aiRes.data.summary << "\"\n";
    }
    else
    {
        std::cout << "AI service error: " << aiRes.fallback_message.value() << "\n";
    }

    std::cout << "\n  Note: This summary is AI-generated from milestone and timesheet data.\n\n";
    ConsoleInput::waitForEnter("Press Enter to go back");
}

ScreenDecorator MyProjectsScreen::decorator() const
{
    return ScreenDecorator("MY PROJECTS")
        .withWidth(ManagerConstants::DEFAULT_PANEL_WIDTH)
        .withPadding(ManagerConstants::DEFAULT_PADDING);
}

void MyProjectsScreen::displayMyProjects(
    const std::vector<std::pair<ProjectDTO, std::string>>& projectsWithHealth)
{
    std::cout << "\n================ MY PROJECTS ================\n";
    std::cout << std::left << std::setw(COL_NO) << "#" << std::setw(COL_PROJECT) << "Project" << std::setw(COL_DATE)
              << "End Date" << std::setw(COL_HEALTH) << "Health" << "\n";
    std::cout << "──────────────────────────────────────────────\n";

    int idx = 1;
    for (const auto& pair : projectsWithHealth)
    {
        const auto& proj = pair.first;
        const auto& health = pair.second;

        std::cout << std::left << std::setw(COL_NO) << idx++ << std::setw(COL_PROJECT)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(proj.name), COL_PROJECT - 1) << std::setw(COL_DATE)
                  << ScreenUtils::valueOrDash(proj.endDate) << std::setw(COL_HEALTH)
                  << ScreenUtils::valueOrDash(health) << "\n";
    }
    std::cout << "──────────────────────────────────────────────\n";
}

void MyProjectsScreen::displayMilestones(const std::vector<MilestoneDTO>& milestones)
{
    std::cout << "Milestones:\n";
    std::cout << std::left << std::setw(COL_NO) << "  #" << std::setw(COL_PROJECT) << "Title" << std::setw(COL_DATE)
              << "Due Date" << std::setw(COL_STATUS) << "Status" << "\n";
    std::cout << "  ────────────────────────────────────────────\n";

    int mIdx = 1;
    for (const auto& ms : milestones)
    {
        std::cout << "  " << std::left << std::setw(COL_NO - 2) << mIdx++ << std::setw(COL_PROJECT)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(ms.title), COL_PROJECT - 1) << std::setw(COL_DATE)
                  << ScreenUtils::valueOrDash(ms.dueDate) << std::setw(COL_STATUS)
                  << ScreenUtils::valueOrDash(ms.status) << "\n";
    }
    std::cout << "\n";
}

void MyProjectsScreen::displayAllocations(
    const std::vector<std::pair<AllocationDTO, std::string>>& allocationsWithName)
{
    std::cout << "Allocated Resources:\n";
    std::cout << std::left << std::setw(22) << "  Name" << std::setw(8) << "%" << std::setw(12)
              << "From" << std::setw(12) << "To" << "\n";
    std::cout << "  ────────────────────────────────────────────\n";

    for (const auto& pair : allocationsWithName)
    {
        const auto& alloc = pair.first;
        const auto& empName = pair.second;

        std::cout << "  " << std::left << std::setw(20)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(empName), 19) << std::setw(8)
                  << (std::to_string(alloc.utilizationPercentage) + "%") << std::setw(12)
                  << ScreenUtils::valueOrDash(alloc.fromDate) << std::setw(12)
                  << ScreenUtils::valueOrDash(alloc.toDate) << "\n";
    }
    std::cout << "\n";
}
