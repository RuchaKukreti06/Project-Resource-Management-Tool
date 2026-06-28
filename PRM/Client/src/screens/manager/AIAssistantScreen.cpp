#include "manager/AIAssistantScreen.h"

#include <iomanip>

#include "api/ApiException.h"
#include "dto/AiResponseDTO.h"
#include "dto/ApiResponse.h"
#include "dto/ProjectDTO.h"
#include "screens/ScreenUtils.h"
#include "services/AiClientService.h"
#include "services/ProjectClientService.h"
#include "utils/ConsoleInput.h"

using namespace ManagerConstants;
using namespace ManagerConstants::AIAssistant;

AIAssistantScreen::AIAssistantScreen(AiClientService& aiService, ProjectClientService& projService,
                                     int currentUserId)
    : aiService_(aiService), projService_(projService), currentUserId_(currentUserId)
{
}

void AIAssistantScreen::displayMenu()
{
    decorator().render();
    std::cout << OPT_SKILL_MATCH
              << ". Skill Match    — Find best employees for a project requirement\n";
    std::cout << OPT_RISK_SUMMARY << ". Risk Summary   — Get a health analysis for a project\n";
    std::cout << OPT_TEAM_BUILDER << ". Team Builder   — Build a whole project team in one go\n";
    std::cout << OPT_BACK << ". Back\n";
}

void AIAssistantScreen::show()
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        displayMenu();
        handleInput();
    }
}

void AIAssistantScreen::handleInput()
{
    std::string choice = ConsoleInput::readLine("Enter option");
    if (choice == OPT_SKILL_MATCH)
    {
        skillMatch();
    }
    else if (choice == OPT_RISK_SUMMARY)
    {
        riskSummary();
    }
    else if (choice == OPT_TEAM_BUILDER)
    {
        teamBuilder();
    }
    else if (choice == OPT_BACK || ScreenUtils::equalsIgnoreCase(choice, "B"))
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1–4.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

void AIAssistantScreen::skillMatch()
{
    try
    {
        std::string reqText = promptSkillRequirement();
        if (reqText.empty())
        {
            showError("Requirement cannot be empty.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }
        auto dto = fetchAIAndHandleFallback(reqText);

        if (!dto.candidates.empty())
        {
            displaySkillMatchResults(dto.candidates);
        }
        else if (!dto.fallback_message.has_value())
        {
            std::cout << "  (No structured results returned by AI)\n\n";
        }
        std::cout << "  Note: AI-generated suggestions. Always verify before allocating.\n\n";

        std::cout << "[" << OPT_GO_ALLOCATE << "] Go to Allocate Resource     [" << OPT_BACK
                  << "] Back\n";
        std::string choice = ConsoleInput::readLine("Choice");
        if (ScreenUtils::equalsIgnoreCase(choice, OPT_GO_ALLOCATE))
        {
            std::cout << "\nPlease navigate to 'Allocate Resource' from the main menu.\n";
            ConsoleInput::waitForEnter("Press Enter to continue\n");
        }
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

std::string AIAssistantScreen::promptSkillRequirement()
{
    std::cout << "\n── Skill Match ────────────────────────────────\n\n";
    return ConsoleInput::readLine("Type what kind of resource you need (e.g. 'Senior C++ dev')");
}

AiSkillMatchResponse AIAssistantScreen::fetchAIAndHandleFallback(const std::string& reqText)
{
    std::cout << "\nSearching... (AI matching in progress)\n";
    AiSkillMatchRequest req;
    req.requirement = reqText;
    auto dto = aiService_.getSkillMatch(req);

    if (dto.fallback_message.has_value())
    {
        std::cout << "  (The AI service provided a plain text response)\n";
        std::cout << "  " << dto.fallback_message.value() << "\n\n";
    }
    return dto;
}

void AIAssistantScreen::riskSummary()
{
    try
    {
        std::cout << "\n── Risk Summary ───────────────────────────────\n\n";
        auto projects = fetchProjectsForRiskSummary();
        if (projects.empty()) return;

        auto selectionOpt = promptForProjectSelection(projects);
        if (!selectionOpt) return;

        const int projectId = projects[selectionOpt.value() - 1].id;
        auto dto = generateAIRiskSummary(projectId);

        if (dto.fallback_message.has_value())
        {
            std::cout << "── AI Risk Summary (Fallback) ───────────────────\n\n";
            std::cout << "  " << dto.fallback_message.value() << "\n\n";
            std::cout << "  Note: AI service encountered an issue generating structured data.\n\n";
            return;
        }

        std::cout << "── AI Risk Summary ──────────────────────────────\n\n";
        std::cout << "\"" << dto.data.summary << "\"\n\n";
        std::cout << "  Note: AI-generated from current milestone and timesheet data.\n\n";
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

std::vector<ProjectDTO> AIAssistantScreen::fetchProjectsForRiskSummary()
{
    auto projResp = projService_.getManagerProjects(currentUserId_);
    if (!projResp.success)
    {
        showError("Could not load projects.");
        return {};
    }

    auto projects = projResp.data;
    if (projects.empty())
    {
        std::cout << "No projects found.\n\n";
        return {};
    }

    std::cout << "Select project:\n";
    int idx = 1;
    for (const auto& proj : projects)
    {
        const std::string hs = proj.healthStatus.empty() ? "ON_TRACK" : proj.healthStatus;
        const std::string icon = (hs == "AT_RISK") ? "🔴" : (hs == "ATTENTION") ? "🟡" : "🟢";
        std::cout << "  " << idx++ << ".  " << icon << " " << proj.name << "\n";
    }
    std::cout << "\n";
    return projects;
}

AiRiskSummaryResponse AIAssistantScreen::generateAIRiskSummary(int projectId)
{
    std::cout << "\nGenerating AI summary...\n\n";
    AiRiskSummaryRequest req;
    req.projectId = projectId;
    return aiService_.getRiskSummary(req);
}

ScreenDecorator AIAssistantScreen::decorator() const
{
    return ScreenDecorator("AI ASSISTANT")
        .withWidth(ManagerConstants::DEFAULT_PANEL_WIDTH)
        .withPadding(ManagerConstants::DEFAULT_PADDING);
}

void AIAssistantScreen::teamBuilder()
{
    try
    {
        std::cout << "\n── Team Builder ───────────────────────────────\n\n";
        std::string reqText =
            ConsoleInput::readLine("Describe your full team requirement in plain English");
            
        if (reqText.empty())
        {
            showError("Requirement cannot be empty.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        std::cout << "\nSearching the entire organization... (calling AI)\n\n";

        AiTeamBuilderRequest req;
        req.requirement = reqText;

        auto dto = aiService_.getTeamBuilder(req);

        if (dto.fallback_message.has_value())
        {
            std::cout << "  (The AI service provided a plain text response)\n";
            std::cout << "  " << dto.fallback_message.value() << "\n\n";
        }

        // ── Output results ──────────────────────────────────────────
        displayTeamMatchResults(dto);

        ConsoleInput::readLine("\nPress Enter to continue");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

void AIAssistantScreen::displaySkillMatchResults(const std::vector<AiCandidateDTO>& candidates)
{
    std::cout << "\n================ AI-MATCHED RESULTS ================\n";
    std::cout << std::left << std::setw(6) << "#" << std::setw(8) << "ID" << std::setw(22)
              << "Employee"
              << "Reason\n";
    std::cout << "─────────────────────────────────────────────────────────\n";

    int idx = 1;
    for (const auto& item : candidates)
    {
        std::string reason = item.reason;

        // Word-wrap reason into lines of 55 chars
        std::vector<std::string> reasonLines;
        while ((int)reason.size() > 55)
        {
            int cut = 55;
            while (cut > 0 && reason[cut] != ' ') cut--;
            if (cut == 0) cut = 55;
            reasonLines.push_back(reason.substr(0, cut));
            reason = reason.substr(cut + 1);
        }
        reasonLines.push_back(reason);

        std::cout << std::left << std::setw(6) << idx++ << std::setw(8) << item.employee_id
                  << std::setw(22) << ScreenUtils::truncate(ScreenUtils::valueOrDash(item.name), 21)
                  << reasonLines[0] << "\n";

        for (size_t i = 1; i < reasonLines.size(); ++i)
        {
            std::cout << std::left << std::setw(6) << " " << std::setw(8) << " " << std::setw(22)
                      << " " << reasonLines[i] << "\n";
        }
        std::cout << "\n";
    }
    std::cout << "─────────────────────────────────────────────────────────\n";
}

void AIAssistantScreen::displayTeamMatchResults(const AiTeamBuilderResponse& dto)
{
    std::cout << "\n================ TEAM MATCH RESULTS ================\n";
    std::cout << std::left << std::setw(20) << "Role" << std::setw(8) << "ID" << std::setw(22)
              << "Employee"
              << "Reason\n";
    std::cout << "─────────────────────────────────────────────────────────\n";

    if (!dto.team.empty())
    {
        for (const auto& item : dto.team)
        {
            printTeamMemberRow(item);
        }
    }
    else if (!dto.fallback_message.has_value())
    {
        std::cout << "  (No structured results returned by AI)\n\n";
    }
    std::cout << "─────────────────────────────────────────────────────────\n";
    std::cout << "  Note: AI-generated suggestions. Verify before allocating.\n\n";
}

void AIAssistantScreen::printTeamMemberRow(const AiTeamMemberDTO& item)
{
    std::string reason = item.reason;
    int empId          = item.employee_id;

    // Word-wrap reason
    std::vector<std::string> reasonLines;
    while ((int)reason.size() > 50)
    {
        int cut = 50;
        while (cut > 0 && reason[cut] != ' ') cut--;
        if (cut == 0) cut = 50;
        reasonLines.push_back(reason.substr(0, cut));
        reason = reason.substr(cut + 1);
    }
    reasonLines.push_back(reason);

    std::cout << std::left
              << std::setw(20) << ScreenUtils::truncate(ScreenUtils::valueOrDash(item.role), 19)
              << std::setw(8) << (empId == 0 ? "-" : std::to_string(empId))
              << std::setw(22) << ScreenUtils::truncate(ScreenUtils::valueOrDash(item.name), 21)
              << reasonLines[0] << "\n";

    for (size_t i = 1; i < reasonLines.size(); ++i)
    {
        std::cout << std::left
                  << std::setw(20) << " "
                  << std::setw(8) << " "
                  << std::setw(22) << " "
                  << reasonLines[i] << "\n";
    }
    std::cout << "\n";
}

std::optional<int> AIAssistantScreen::promptForProjectSelection(
    const std::vector<ProjectDTO>& projects)
{
    std::string pNum = ConsoleInput::readLine("Enter project number (or 0 to cancel)");
    auto parsedSelection = ScreenUtils::safeParseInt(pNum);
    if (!parsedSelection)
    {
        showError("Invalid selection format");
        return std::nullopt;
    }
    int selection = parsedSelection.value();
    if (selection < 1 || selection > static_cast<int>(projects.size()))
    {
        return std::nullopt;
    }
    return selection;
}
