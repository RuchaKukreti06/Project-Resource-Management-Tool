#include "manager/AIAssistantScreen.h"
#include "manager/AllocateResourceScreen.h"
#include "AuthSession.h"
#include <iomanip>
#include <nlohmann/json.hpp>
#include "dto/AiResponseDTO.h"
#include "dto/ProjectDTO.h"
#include "dto/ApiResponse.h"

AIAssistantScreen::AIAssistantScreen()
{
}

void AIAssistantScreen::displayMenu()
{
    decorator().render();
    std::cout << "1. Skill Match    — Find best employees for a project requirement\n";
    std::cout << "2. Risk Summary   — Get a health analysis for a project\n";
    std::cout << "3. Team Builder   — Build a whole project team in one go\n";
    std::cout << "4. Back\n";
}

void AIAssistantScreen::show(ApiClient& apiClient)
{
    while (true)
    {
        displayMenu();
        std::string choice = ScreenUtils::readLine("Enter option");
        if (choice == "1")
        {
            skillMatch(apiClient);
        }
        else if (choice == "2")
        {
            riskSummary(apiClient);
        }
        else if (choice == "3")
        {
            teamBuilder(apiClient);
        }
        else if (choice == "4" || choice == "B" || choice == "b")
        {
            break;
        }
        else
        {
            showError("Invalid option. Please enter 1–4.");
        }
    }
}

void AIAssistantScreen::handleInput(ApiClient& apiClient)
{
}

void AIAssistantScreen::skillMatch(ApiClient& apiClient)
{
    try
    {
        std::cout << "\n── Skill Match ────────────────────────────────\n\n";
        std::string reqText = ScreenUtils::readLine("Type what kind of resource you need (e.g. 'Senior C++ dev')");

        std::cout << "\nSearching... (AI matching in progress)\n";

        AiSkillMatchRequest req;
        req.requirement = reqText;
        auto response = apiClient.post("/ai/skill-match", req.toJson());

        AiSkillMatchResponse dto = AiSkillMatchResponse::fromJson(response);

        if (dto.fallback_message.has_value())
        {
            std::cout << "  (The AI service provided a plain text response)\n";
            std::cout << "  " << dto.fallback_message.value() << "\n\n";
        }

        // ── Table header ──────────────────────────────────────────
        const int W_NO     = 4;
        const int W_ID     = 6;
        const int W_NAME   = 22;
        const int W_REASON = 55;
        std::string divider(W_NO + W_ID + W_NAME + W_REASON + 3, '-');

        std::cout << "\nAI-MATCHED RESULTS\n";
        std::cout << divider << "\n";
        std::cout << std::left
                  << std::setw(W_NO)   << "#"
                  << std::setw(W_ID)   << "ID"
                  << std::setw(W_NAME) << "Employee"
                  << "Reason\n";
        std::cout << divider << "\n";

        if (!dto.candidates.empty())
        {
            int idx = 1;
            for (const auto& item : dto.candidates)
            {
                std::string reason = item.reason;

                // Word-wrap reason into lines of W_REASON chars
                std::vector<std::string> reasonLines;
                while ((int)reason.size() > W_REASON)
                {
                    int cut = W_REASON;
                    while (cut > 0 && reason[cut] != ' ') cut--;
                    if (cut == 0) cut = W_REASON;
                    reasonLines.push_back(reason.substr(0, cut));
                    reason = reason.substr(cut + 1);
                }
                reasonLines.push_back(reason);

                // First line — print all columns
                std::cout << std::left
                          << std::setw(W_NO)   << idx++
                          << std::setw(W_ID)   << item.employee_id
                          << std::setw(W_NAME) << item.name.substr(0, W_NAME - 1)
                          << reasonLines[0] << "\n";

                // Continuation lines — indent to reason column
                std::string indent(W_NO + W_ID + W_NAME, ' ');
                for (size_t i = 1; i < reasonLines.size(); ++i)
                    std::cout << indent << reasonLines[i] << "\n";

                std::cout << "\n";
            }
        }
        else if (!dto.fallback_message.has_value())
        {
            std::cout << "  (No structured results returned by AI)\n\n";
        }

        std::cout << divider << "\n";
        std::cout << "  Note: AI-generated suggestions. Always verify before allocating.\n\n";

        std::cout << "[A] Go to Allocate Resource     [B] Back\n";
        std::string choice = ScreenUtils::readLine("Choice");
        if (choice == "A" || choice == "a")
        {
            AllocateResourceScreen().show(apiClient);
        }
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error during skill matching: ") + ex.what());
    }
}

void AIAssistantScreen::riskSummary(ApiClient& apiClient)
{
    try
    {
        std::cout << "\n── Risk Summary ───────────────────────────────\n\n";

        int managerId  = api::AuthSession::instance().userId();
        auto projResp  = ApiListResponse<ProjectDTO>::fromJson(apiClient.get("/managers/" + std::to_string(managerId) + "/projects"));
        if (!projResp.success)
        {
            showError("Could not load projects.");
            return;
        }

        auto projects = projResp.data;
        if (projects.empty())
        {
            std::cout << "No projects found.\n\n";
            return;
        }

        std::cout << "Select project:\n";
        int idx = 1;
        for (const auto& proj : projects)
        {
            const std::string hs   = proj.healthStatus.empty() ? "ON_TRACK" : proj.healthStatus;
            const std::string icon  = (hs == "AT_RISK") ? "🔴" : (hs == "ATTENTION") ? "🟡" : "🟢";
            std::cout << "  " << idx++ << ".  " << icon << " "
                      << proj.name << "\n";
        }

        std::cout << "\n";
        std::string pNum = ScreenUtils::readLine("Enter project number (or 0 to cancel)");
        auto parsedSelection = ScreenUtils::safeParseInt(pNum);
        if (!parsedSelection) throw std::invalid_argument("Invalid selection format");
        int selection = parsedSelection.value();
        if (selection < 1 || selection > static_cast<int>(projects.size()))
            return;

        const int projectId = projects[selection - 1].id;

        std::cout << "\nGenerating AI summary...\n\n";

        AiRiskSummaryRequest req;
        req.projectId = projectId;
        auto response          = apiClient.post("/ai/risk-summary", req.toJson());

        AiRiskSummaryResponse dto = AiRiskSummaryResponse::fromJson(response);

        if (dto.fallback_message.has_value())
        {
            std::cout << "── AI Risk Summary (Fallback) ───────────────────\n\n";
            std::cout << "  " << dto.fallback_message.value() << "\n\n";
            std::cout << "  Note: AI service encountered an issue generating structured data.\n\n";
            return;
        }

        const std::string summary = dto.data.summary;
        std::cout << "── AI Risk Summary ──────────────────────────────\n\n";
        std::cout << "\"" << summary << "\"\n\n";
        std::cout << "  Note: AI-generated from current milestone and timesheet data.\n\n";
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error generating risk summary: ") + ex.what());
    }
}

ScreenDecorator AIAssistantScreen::decorator() const
{
    return ScreenDecorator("AI ASSISTANT").withWidth(40).withPadding(2);
}

void AIAssistantScreen::teamBuilder(ApiClient& apiClient)
{
    try
    {
        std::cout << "\n── Team Builder ───────────────────────────────\n\n";
        std::string reqText =
            ScreenUtils::readLine("Describe your full team requirement in plain English");

        std::cout << "\nSearching the entire organization... (calling AI)\n\n";

        nlohmann::json payload = {{"requirement", reqText}};
        auto response          = apiClient.post("/ai/team-builder", payload);

        AiTeamBuilderResponse dto = AiTeamBuilderResponse::fromJson(response);

        if (dto.fallback_message.has_value())
        {
            std::cout << "  (The AI service provided a plain text response)\n";
            std::cout << "  " << dto.fallback_message.value() << "\n\n";
        }

        // ── Output results ──────────────────────────────────────────
        displayTeamMatchResults(dto);

        ScreenUtils::readLine("\nPress Enter to continue");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error during team building: ") + ex.what());
    }
}

void AIAssistantScreen::displayTeamMatchResults(const AiTeamBuilderResponse& dto)
{
    const int W_ROLE   = 20;
    const int W_ID     = 6;
    const int W_NAME   = 22;
    const int W_REASON = 50;
    std::string divider(W_ROLE + W_ID + W_NAME + W_REASON + 3, '-');

    std::cout << "\nTEAM MATCH RESULTS\n";
    std::cout << divider << "\n";
    std::cout << std::left
              << std::setw(W_ROLE) << "Role"
              << std::setw(W_ID)   << "ID"
              << std::setw(W_NAME) << "Employee"
              << "Reason\n";
    std::cout << divider << "\n";

    if (!dto.team.empty())
    {
        for (const auto& item : dto.team)
        {
            std::string role   = item.role.substr(0, W_ROLE - 1);
            std::string reason = item.reason;
            int empId          = item.employee_id;
            std::string name   = item.name.substr(0, W_NAME - 1);

            // Word-wrap reason
            std::vector<std::string> reasonLines;
            while ((int)reason.size() > W_REASON)
            {
                int cut = W_REASON;
                while (cut > 0 && reason[cut] != ' ') cut--;
                if (cut == 0) cut = W_REASON;
                reasonLines.push_back(reason.substr(0, cut));
                reason = reason.substr(cut + 1);
            }
            reasonLines.push_back(reason);

            std::cout << std::left
                      << std::setw(W_ROLE) << role
                      << std::setw(W_ID)   << (empId == 0 ? "-" : std::to_string(empId))
                      << std::setw(W_NAME) << name
                      << reasonLines[0] << "\n";

            std::string indent(W_ROLE + W_ID + W_NAME, ' ');
            for (size_t i = 1; i < reasonLines.size(); ++i)
                std::cout << indent << reasonLines[i] << "\n";

            std::cout << "\n";
        }
    }
    else if (!dto.fallback_message.has_value())
    {
        std::cout << "  (No structured results returned by AI)\n\n";
    }

    std::cout << divider << "\n";
    std::cout << "  Note: AI-generated suggestions. Verify before allocating.\n\n";
}
