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
        showError(Messages::SOMETHING_WENT_WRONG);
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

MyProjectsScreen::ProjectHealthSummary MyProjectsScreen::buildProjectHealthSummary(
    const std::vector<MilestoneDTO>& milestones)
{
    ProjectHealthSummary summary;
    summary.healthStatus = HealthStatus::ON_TRACK;

    bool hasOverdue = false;
    bool hasApproaching = false;

    std::string todayStr = DateUtils::getCurrentDateYYYYMMDD();
    std::string nextStr = DateUtils::getDateNextWeekYYYYMMDD();

    for (const auto& milestone : milestones)
    {
        if (milestone.status == "DONE" || milestone.dueDate.empty())
        {
            continue;
        }

        if (milestone.dueDate < todayStr)
        {
            hasOverdue = true;
            summary.overdueTitles.push_back(milestone.title);
        }
        else if (milestone.dueDate <= nextStr)
        {
            hasApproaching = true;
        }
    }

    if (hasOverdue)
    {
        summary.healthStatus = HealthStatus::AT_RISK;
    }
    else if (hasApproaching)
    {
        summary.healthStatus = HealthStatus::ATTENTION;
    }

    return summary;
}

std::string MyProjectsScreen::calculateProjectHealth(int projectId)
{
    auto milestoneResponse = projService_.getProjectMilestones(projectId);
    if (!milestoneResponse.success)
    {
        return HealthStatus::ON_TRACK;
    }
    return buildProjectHealthSummary(milestoneResponse.data).healthStatus;
}

std::optional<int> MyProjectsScreen::promptForProjectSelection(size_t maxSelection)
{
    std::string selStr =
        ConsoleInput::readLine("Select project number to view details (or 0 to go back)");
    auto parsedSel = ScreenUtils::safeParseInt(selStr);
    if (!parsedSel)
    {
        showError(Messages::INVALID_SELECTION_FORMAT);
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
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
            auto projectResponse = projService_.getProject(projectId);
            if (!projectResponse.success || !projectResponse.data) return;
            auto proj = *projectResponse.data;

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
            else
            {
                showError(Messages::INVALID_OPTION);
                ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
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
            showError(Messages::SOMETHING_WENT_WRONG);
            break;
        }
    }
}

void MyProjectsScreen::displayMilestoneRisks(int projectId)
{
    auto milestoneResponse = projService_.getProjectMilestones(projectId);
    if (!milestoneResponse.success)
    {
        return;
    }

    auto summary = buildProjectHealthSummary(milestoneResponse.data);

    std::cout << "Health Status : " << summary.healthStatus << "\n\n";
    if (!summary.overdueTitles.empty())
    {
        std::cout << "Risk Flags:\n";
        for (const auto& title : summary.overdueTitles)
        {
            std::cout << "  ✗ " << title << " milestone is overdue\n";
        }
        std::cout << "\n";
    }

    std::vector<MilestoneDTO> milestones = milestoneResponse.success ? milestoneResponse.data : std::vector<MilestoneDTO>{};
    displayMilestones(milestones);
}

void MyProjectsScreen::displayAllocationsDetail(int projectId)
{
    auto allocationsResponse = allocService_.getProjectAllocations(projectId);

    std::vector<std::pair<AllocationDTO, std::string>> allocData;
    std::map<int, std::string> employeeNameById;
    auto employeesResponse = empService_.getTeamEmployees(currentUserId_);
    if (employeesResponse.success)
    {
        for (const auto& e : employeesResponse.data)
        {
            employeeNameById[e.id] = e.fullName;
        }
    }

    for (const auto& alloc : (allocationsResponse.success ? allocationsResponse.data : std::vector<AllocationDTO>{}))
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
    auto aiResponse = aiService_.getRiskSummary(req);

    std::cout << "\n── AI Risk Summary — " << projectName << " ────────────\n\n";

    if (!aiResponse.fallback_message.has_value())
    {
        std::cout << "\"" << aiResponse.data.summary << "\"\n";
    }
    else
    {
        std::cout << "AI service error: " << aiResponse.fallback_message.value() << "\n";
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
