#include "manager/AllocateResourceScreen.h"
#include "AuthSession.h"
#include "dto/ApiResponse.h"
#include "dto/ProjectDTO.h"
#include "dto/EmployeeDTO.h"
#include "dto/AllocationDTO.h"
#include "dto/AiResponseDTO.h"
#include <ctime>
#include "api/ApiException.h"
#include "services/AiClientService.h"
#include "services/ProjectClientService.h"
#include "services/AllocationClientService.h"
#include "services/EmployeeClientService.h"

AllocateResourceScreen::AllocateResourceScreen(AiClientService& aiService, AllocationClientService& allocService, ProjectClientService& projService, EmployeeClientService& empService)
    : aiService_(aiService), allocService_(allocService), projService_(projService), empService_(empService)
{
}

void AllocateResourceScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    std::cout << "1. Find resource using AI (recommended)\n";
    std::cout << "2. Allocate directly (I already know who I want)\n";
    std::cout << "3. End an existing allocation\n";
    std::cout << "4. Back\n";
}

void AllocateResourceScreen::show()
{
    while (true)
    {
        displayMenu();
        handleInput();
        break; // Return after one action or back
    }
}

void AllocateResourceScreen::handleInput()
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        findResourceAI();
    }
    else if (choice == "2")
    {
        allocateDirectly();
    }
    else if (choice == "3")
    {
        endAllocation();
    }
    else if (choice == "4" || choice == "B" || choice == "b")
    {
        return;
    }
    else
    {
        showError("Invalid option. Please enter 1–4.");
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void AllocateResourceScreen::findResourceAI()
{
    try
    {
        std::cout << "\nStep 1 — Select Project\n";
        std::string projectInput = ScreenUtils::readLine("Enter project name or ID");

        std::cout << "\nStep 2 — Describe your requirement\n";
        std::string reqText = ScreenUtils::readLine("Type what kind of resource you need");

        std::cout << "\nSearching... (AI matching in progress)\n";
        
        AiSkillMatchRequest req;
        req.requirement = reqText;
        auto aiRes = aiService_.getSkillMatch(req);
        if (aiRes.fallback_message.has_value())
        {
            showError("AI service error: " + aiRes.fallback_message.value());
            return;
        }

        auto candidates = aiRes.candidates;
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

        if (!candidates.empty())
        {
            int idx = 1;
            for (const auto& item : candidates)
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

        std::cout << divider << "\n";
        std::cout << "  Note: Suggestions are AI-generated. Verify before confirming.\n\n";

        if (candidates.empty())
        {
            showError("No candidates available or AI returned plain text.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        std::string selStr = ScreenUtils::readLine("Select employee (enter #, or 0 to search again)");
        auto parsedSel = ScreenUtils::safeParseInt(selStr);
        if (!parsedSel) throw std::invalid_argument("Invalid selection format");
        int selection = parsedSel.value();
        if (selection < 1 || selection > (int)candidates.size())
        {
            return;
        }

        auto selectedEmp = candidates[selection - 1];
        int empId = selectedEmp.employee_id;

        std::cout << "\n── " << selectedEmp.name << " ─────────────────────────────────\n";
        std::cout << "Current Utilisation: 0%   (fully on bench)\n\n";

        std::string utilStr = ScreenUtils::readLine("Set Allocation Utilisation %");
        auto parsedUtil = ScreenUtils::safeParseInt(utilStr);
        if (!parsedUtil) throw std::invalid_argument("Invalid utilisation format");
        int utilPercent = parsedUtil.value();
        std::string fromDate = ScreenUtils::readLine("From Date (YYYY-MM-DD)");
        std::string toDate = ScreenUtils::readLine("To Date (YYYY-MM-DD)");

        // Find Project ID
        int projectId = 0;
        auto projRes = projService_.viewAllProjects();
        if (projRes.success)
        {
            for (const auto& p : projRes.data)
            {
                if (std::to_string(p.id) == projectInput || p.name == projectInput)
                {
                    projectId = p.id;
                    break;
                }
            }
        }

        if (projectId == 0)
        {
            showError("Project not found.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        CreateAllocationRequest allocReq;
        allocReq.employeeId = empId;
        allocReq.projectId = projectId;
        allocReq.utilizationPercentage = utilPercent;
        allocReq.fromDate = fromDate;
        allocReq.toDate = toDate;

        auto allocRes = allocService_.createAllocation(allocReq);

        if (allocRes.success)
        {
            showSuccess("Allocation saved successfully! ✓");
        }
        else
        {
            showError(allocRes.message);
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

void AllocateResourceScreen::allocateDirectly()
{
    try
    {
        std::string projInput = ScreenUtils::readLine("Select Project (Enter name or ID)");
        std::string empIdStr = ScreenUtils::readLine("Enter Employee ID");
        auto parsedEmp = ScreenUtils::safeParseInt(empIdStr);
        if (!parsedEmp) throw std::invalid_argument("Invalid employee ID");
        int empIdParsed = parsedEmp.value();

        // Find Project ID
        int projectId = 0;
        auto projRes = projService_.viewAllProjects();
        if (projRes.success)
        {
            for (const auto& p : projRes.data)
            {
                if (std::to_string(p.id) == projInput || p.name == projInput)
                {
                    projectId = p.id;
                    break;
                }
            }
        }

        if (projectId == 0)
        {
            showError("Project not found.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        std::string utilStr = ScreenUtils::readLine("Utilisation %");
        auto parsedUtil = ScreenUtils::safeParseInt(utilStr);
        if (!parsedUtil) throw std::invalid_argument("Invalid utilisation format");
        int utilPercent = parsedUtil.value();
        std::string fromDate = ScreenUtils::readLine("From Date (YYYY-MM-DD)");
        std::string toDate = ScreenUtils::readLine("To Date (YYYY-MM-DD)");

        CreateAllocationRequest allocReq;
        allocReq.employeeId = empIdParsed;
        allocReq.projectId = projectId;
        allocReq.utilizationPercentage = utilPercent;
        allocReq.fromDate = fromDate;
        allocReq.toDate = toDate;

        auto allocRes = allocService_.createAllocation(allocReq);

        if (allocRes.success)
        {
            showSuccess("Allocation saved successfully. ✓");
        }
        else
        {
            showError(allocRes.message);
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

void AllocateResourceScreen::endAllocation()
{
    try
    {
        std::string projInput = ScreenUtils::readLine("Select Project (Enter name or ID)");

        int projectId = 0;
        auto projRes = projService_.viewAllProjects();
        if (projRes.success)
        {
            for (const auto& p : projRes.data)
            {
                if (std::to_string(p.id) == projInput || p.name == projInput)
                {
                    projectId = p.id;
                    break;
                }
            }
        }

        if (projectId == 0)
        {
            showError("Project not found.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto allocRes = allocService_.getProjectAllocations(projectId);
        
        // clearScreen();
        std::cout << "\nActive Allocations on this project:\n";
        std::cout << std::left << std::setw(6) << "#"
                  << std::setw(20) << "Employee"
                  << std::setw(8) << "%"
                  << std::setw(12) << "From"
                  << std::setw(12) << "To" << "\n";
        ScreenUtils::printDivider();

        std::vector<AllocationDTO> activeAllocs;
        int idx = 1;
        if (!allocRes.success || allocRes.data.empty())
        {
            showInfo("No active allocations found on this project.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }
        for (const auto& alloc : allocRes.data)
        {
            activeAllocs.push_back(alloc);
            int empId = alloc.employeeId;
            
            // fetch emp name
            std::string empName = "Emp " + std::to_string(empId);
            auto empRes = empService_.viewAllEmployees();
            if (empRes.success)
            {
                for (const auto& e : empRes.data)
                {
                    if (e.id == empId)
                    {
                        empName = e.fullName;
                        break;
                    }
                }
            }

            std::cout << std::left << std::setw(6) << idx++
                      << std::setw(20) << empName.substr(0, 19)
                      << std::setw(8) << (std::to_string(alloc.utilizationPercentage) + "%")
                      << std::setw(12) << alloc.fromDate
                      << std::setw(12) << alloc.toDate << "\n";
        }
        ScreenUtils::printDivider();

        if (activeAllocs.empty())
        {
            showInfo("No active allocations found on this project.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        std::string selStr = ScreenUtils::readLine("Select resource to deallocate (enter #, or 0 to cancel)");
        auto parsedSel = ScreenUtils::safeParseInt(selStr);
        if (!parsedSel) throw std::invalid_argument("Invalid selection format");
        int selection = parsedSel.value();
        if (selection < 1 || selection > (int)activeAllocs.size())
        {
            return;
        }

        auto targetAlloc = activeAllocs[selection - 1];
        int allocId = targetAlloc.id;

        std::cout << "Confirm setting end date to today (Y/N): ";
        std::string confirm = ScreenUtils::readLine("Choice");
        if (confirm == "Y" || confirm == "y")
        {
            std::time_t now = std::time(nullptr);
            std::tm local = {};
#ifdef _WIN32
            localtime_s(&local, &now);
#else
            local = *std::localtime(&now);
#endif
            char buffer[11] = {0};
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local);

            auto response = allocService_.endAllocation(allocId, std::string(buffer));

            if (response.success)
            {
                showSuccess("Allocation ended successfully. ✓");
            }
            else
            {
                showError(response.message);
            }
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

ScreenDecorator AllocateResourceScreen::decorator() const
{
    return ScreenDecorator("ALLOCATE RESOURCE").withWidth(40).withPadding(2);
}
