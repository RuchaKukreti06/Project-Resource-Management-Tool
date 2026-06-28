#include "manager/AllocateResourceScreen.h"
#include <map>
#include "dto/ApiResponse.h"
#include "dto/ProjectDTO.h"
#include "dto/EmployeeDTO.h"
#include "dto/AllocationDTO.h"
#include "dto/AiResponseDTO.h"
#include "api/ApiException.h"
#include "services/AiClientService.h"
#include "services/ProjectClientService.h"
#include "services/AllocationClientService.h"
#include "services/EmployeeClientService.h"
#include "screens/ScreenUtils.h"
#include "utils/DateUtils.h"
#include "utils/ConsoleInput.h"
#include <iomanip>

using namespace ManagerConstants;
using namespace ManagerConstants::AllocateResource;

AllocateResourceScreen::AllocateResourceScreen(AiClientService& aiService, AllocationClientService& allocService, ProjectClientService& projService, EmployeeClientService& empService, int currentUserId)
    : aiService_(aiService), allocService_(allocService), projService_(projService), empService_(empService), currentUserId_(currentUserId)
{
}

void AllocateResourceScreen::displayMenu()
{
    decorator().render();
    std::cout << OPT_FIND_RESOURCE_AI << ". Find resource using AI (recommended)\n";
    std::cout << OPT_ALLOCATE_DIRECTLY << ". Allocate directly (I already know who I want)\n";
    std::cout << OPT_END_ALLOCATION << ". End an existing allocation\n";
    std::cout << OPT_BACK << ". Back\n";
}

void AllocateResourceScreen::show()
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        displayMenu();
        handleInput();
    }
}

void AllocateResourceScreen::handleInput()
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == OPT_FIND_RESOURCE_AI)
    {
        findResourceAI();
    }
    else if (choice == OPT_ALLOCATE_DIRECTLY)
    {
        allocateDirectly();
    }
    else if (choice == OPT_END_ALLOCATION)
    {
        endAllocation();
    }
    else if (choice == OPT_BACK || ScreenUtils::equalsIgnoreCase(choice, "B"))
    {
        keepRunning_ = false;
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
        auto projectIdOpt = promptForProject("Enter project name or ID");
        if (!projectIdOpt) return;
        int projectId = projectIdOpt.value();

        auto reqTextOpt = promptForAIRequirement();
        if (!reqTextOpt) return;
        
        auto candidateOpt = fetchSkillMatchCandidates(reqTextOpt.value());
        if (!candidateOpt) return;
        
        int empId = candidateOpt.value().employee_id;

        promptForAllocationDetailsAndSave(empId, projectId);
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

std::optional<AiCandidateDTO> AllocateResourceScreen::fetchSkillMatchCandidates(const std::string& reqText)
{
    std::cout << "\nSearching... (AI matching in progress)\n";
    AiSkillMatchRequest req;
    req.requirement = reqText;
    auto aiRes = aiService_.getSkillMatch(req);
    
    if (aiRes.fallback_message.has_value())
    {
        showError("AI service error: " + aiRes.fallback_message.value());
        return std::nullopt;
    }

    return displayAndSelectAICandidate(aiRes.candidates);
}

void AllocateResourceScreen::allocateDirectly()
{
    try
    {
        auto projectIdOpt = promptForProject("Select Project (Enter name or ID)");
        if (!projectIdOpt) return;
        int projectId = projectIdOpt.value();

        auto empIdOpt = promptForEmployeeId();
        if (!empIdOpt) return;
        int empId = empIdOpt.value();

        promptForAllocationDetailsAndSave(empId, projectId);
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
        auto projectIdOpt = promptForProject("Select Project (Enter name or ID)");
        if (!projectIdOpt) return;
        int projectId = projectIdOpt.value();

        auto allocOpt = selectActiveAllocation(projectId);
        if (!allocOpt) return;
        
        confirmAndEndAllocation(allocOpt.value().id);
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
    return ScreenDecorator("ALLOCATE RESOURCE")
        .withWidth(ManagerConstants::DEFAULT_PANEL_WIDTH)
        .withPadding(ManagerConstants::DEFAULT_PADDING);
}

void AllocateResourceScreen::displayCandidates(const std::vector<AiCandidateDTO>& candidates)
{
    std::cout << "\n================ AI-MATCHED RESULTS ================\n";
    std::cout << std::left
              << std::setw(6) << "#"
              << std::setw(8) << "ID"
              << std::setw(22) << "Employee"
              << "Reason\n";
    std::cout << "─────────────────────────────────────────────────────────\n";

    int idx = 1;
    for (const auto& item : candidates)
    {
        printCandidateRow(item, idx++);
    }
    std::cout << "─────────────────────────────────────────────────────────\n";
}

void AllocateResourceScreen::printCandidateRow(const AiCandidateDTO& item, int idx)
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

    std::cout << std::left
              << std::setw(6) << idx
              << std::setw(8) << item.employee_id
              << std::setw(22) << ScreenUtils::truncate(ScreenUtils::valueOrDash(item.name), 21)
              << reasonLines[0] << "\n";
              
    for (size_t i = 1; i < reasonLines.size(); ++i)
    {
        std::cout << std::left
                  << std::setw(6) << " "
                  << std::setw(8) << " "
                  << std::setw(22) << " "
                  << reasonLines[i] << "\n";
    }
    std::cout << "\n";
}

void AllocateResourceScreen::displayActiveAllocations(const std::vector<std::pair<AllocationDTO, std::string>>& allocationsWithName)
{
    std::cout << "\n================ ACTIVE ALLOCATIONS ================\n";
    std::cout << std::left
              << std::setw(6) << "#"
              << std::setw(20) << "Employee"
              << std::setw(8) << "%"
              << std::setw(12) << "From"
              << std::setw(12) << "To" << "\n";
    std::cout << "────────────────────────────────────────────────────────\n";

    int idx = 1;
    for (const auto& pair : allocationsWithName)
    {
        const auto& alloc = pair.first;
        const auto& empName = pair.second;

        std::cout << std::left
                  << std::setw(6) << idx++
                  << std::setw(20) << ScreenUtils::truncate(ScreenUtils::valueOrDash(empName), 19)
                  << std::setw(8) << (std::to_string(alloc.utilizationPercentage) + "%")
                  << std::setw(12) << ScreenUtils::valueOrDash(alloc.fromDate)
                  << std::setw(12) << ScreenUtils::valueOrDash(alloc.toDate) << "\n";
    }
    std::cout << "────────────────────────────────────────────────────────\n";
}

std::optional<std::string> AllocateResourceScreen::promptForAIRequirement()
{
    std::cout << "\nStep 2 — Describe your requirement\n";
    std::string reqText = ScreenUtils::readLine("Type what kind of resource you need");
    if (reqText.empty()) return std::nullopt;
    return reqText;
}

std::optional<int> AllocateResourceScreen::promptForProject(const std::string& promptText)
{
    std::string projInput = ScreenUtils::readLine(promptText);
    if (projInput.empty()) return std::nullopt;

    int projectId = 0;
    auto projRes = projService_.getManagerProjects(currentUserId_);
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
        showError("Project not found or you are not authorized to manage it.");
        ScreenUtils::readLine("Press Enter to continue");
        return std::nullopt;
    }
    return projectId;
}

std::optional<AiCandidateDTO> AllocateResourceScreen::displayAndSelectAICandidate(const std::vector<AiCandidateDTO>& candidates)
{
    displayCandidates(candidates);
    std::cout << "  Note: Suggestions are AI-generated. Verify before confirming.\n\n";

    if (candidates.empty())
    {
        showError("No candidates available or AI returned plain text.");
        ScreenUtils::readLine("Press Enter to continue");
        return std::nullopt;
    }

    std::string selStr = ScreenUtils::readLine("Select employee (enter #, or 0 to search again)");
    auto parsedSel = ScreenUtils::safeParseInt(selStr);
    if (!parsedSel) 
    {
        showError("Invalid selection format.");
        return std::nullopt;
    }
    int selection = parsedSel.value();
    if (selection < 1 || selection > (int)candidates.size())
    {
        return std::nullopt;
    }

    return candidates[selection - 1];
}

void AllocateResourceScreen::promptForAllocationDetailsAndSave(int empId, int projectId)
{
    std::cout << "\n── Selected Employee ID: " << empId << " ─────────────────────────────\n";
    std::string utilStr = ScreenUtils::readLine("Set Allocation Utilisation %");
    auto parsedUtil = ScreenUtils::safeParseInt(utilStr);
    if (!parsedUtil || parsedUtil.value() < 1 || parsedUtil.value() > 100) 
    {
        showError("Utilization must be between 1 and 100.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    int utilPercent = parsedUtil.value();
    std::string fromDate = ScreenUtils::readLine("From Date (YYYY-MM-DD)");
    if (fromDate.empty()) {
        showError("From Date is required.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }
    std::string fromDateError = DateUtils::validateDateYYYYMMDD(fromDate, true);
    if (!fromDateError.empty()) {
        showError(fromDateError);
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return;
    }

    std::string toDate = ScreenUtils::readLine("To Date (YYYY-MM-DD) [Optional]");
    if (!toDate.empty()) {
        std::string toDateError = DateUtils::validateDateYYYYMMDD(toDate, true);
        if (!toDateError.empty()) {
            showError(toDateError);
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }
        if (toDate < fromDate) {
            showError("To Date cannot be before From Date.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }
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

std::optional<int> AllocateResourceScreen::promptForEmployeeId()
{
    std::string empIdStr = ScreenUtils::readLine("Enter Employee ID");
    auto parsedEmp = ScreenUtils::safeParseInt(empIdStr);
    if (!parsedEmp) 
    {
        showError("Invalid employee ID");
        return std::nullopt;
    }
    return parsedEmp.value();
}

std::optional<AllocationDTO> AllocateResourceScreen::selectActiveAllocation(int projectId)
{
    auto allocRes = allocService_.getProjectAllocations(projectId);
    

    if (!allocRes.success || allocRes.data.empty())
    {
        showInfo("No active allocations found on this project.");
        ScreenUtils::readLine("Press Enter to continue");
        return std::nullopt;
    }
    
    auto activeAllocs = allocRes.data;
    auto allocData = fetchAllocationsWithNames(activeAllocs);
    displayActiveAllocations(allocData);

    return promptAndReturnAllocation(activeAllocs);
}

std::vector<std::pair<AllocationDTO, std::string>> AllocateResourceScreen::fetchAllocationsWithNames(const std::vector<AllocationDTO>& activeAllocs)
{
    std::vector<std::pair<AllocationDTO, std::string>> allocationsWithNames;
    
    std::map<int, std::string> employeeNameById;
    auto teamResponse = empService_.getTeamEmployees(currentUserId_);
    if (teamResponse.success)
    {
        for (const auto& employee : teamResponse.data)
        {
            employeeNameById[employee.id] = employee.fullName;
        }
    }

    for (const auto& allocation : activeAllocs)
    {
        std::string employeeName = "Emp " + std::to_string(allocation.employeeId);
        auto it = employeeNameById.find(allocation.employeeId);
        if (it != employeeNameById.end())
        {
            employeeName = it->second;
        }

        allocationsWithNames.push_back({allocation, employeeName});
    }

    return allocationsWithNames;
}

std::optional<AllocationDTO> AllocateResourceScreen::promptAndReturnAllocation(const std::vector<AllocationDTO>& activeAllocs)
{
    std::string selStr = ScreenUtils::readLine("Select resource to deallocate (enter #, or 0 to cancel)");
    auto parsedSel = ScreenUtils::safeParseInt(selStr);
    if (!parsedSel) 
    {
        showError("Invalid selection format");
        return std::nullopt;
    }
    int selection = parsedSel.value();
    if (selection < 1 || selection > (int)activeAllocs.size())
    {
        return std::nullopt;
    }

    return activeAllocs[selection - 1];
}

void AllocateResourceScreen::confirmAndEndAllocation(int allocId)
{
    std::cout << "Confirm setting end date to today (Y/N): ";
    std::string confirm = ScreenUtils::readLine("Choice");
    if (confirm == "Y" || confirm == "y")
    {
        std::string buffer = DateUtils::getCurrentDateYYYYMMDD();

        auto response = allocService_.endAllocation(allocId, buffer);

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
