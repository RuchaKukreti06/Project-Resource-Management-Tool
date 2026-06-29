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
    if (choice == OPT_ALLOCATE_DIRECTLY)
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
        showError(Messages::INVALID_OPTION);
        ScreenUtils::readLine(Messages::PRESS_ENTER_TO_CONTINUE);
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

        createAllocation(empId, projectId);
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine(Messages::PRESS_ENTER_TO_CONTINUE);
    }
    catch (const std::exception&)
    {
        showError(Messages::SOMETHING_WENT_WRONG);
    }
}

std::optional<AiCandidateDTO> AllocateResourceScreen::fetchSkillMatchCandidates(const std::string& reqText)
{
    std::cout << "\nSearching... (AI matching in progress)\n";
    AiSkillMatchRequest req;
    req.requirement = reqText;
    auto aiResponse = aiService_.getSkillMatch(req);
    
    if (aiResponse.fallback_message.has_value())
    {
        showError("AI service error: " + aiResponse.fallback_message.value());
        return std::nullopt;
    }

    return selectAICandidate(aiResponse.candidates);
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

        createAllocation(empId, projectId);
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine(Messages::PRESS_ENTER_TO_CONTINUE);
    }
    catch (const std::exception&)
    {
        showError(Messages::SOMETHING_WENT_WRONG);
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
        
        endAllocation(allocOpt.value().id);
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine(Messages::PRESS_ENTER_TO_CONTINUE);
    }
    catch (const std::exception&)
    {
        showError(Messages::SOMETHING_WENT_WRONG);
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
    
    std::vector<std::string> reasonLines;
    while ((int)reason.size() > Display::WRAP_WIDTH_AI_REASON)
    {
        int cut = Display::WRAP_WIDTH_AI_REASON;
        while (cut > 0 && reason[cut] != ' ') cut--;
        if (cut == 0) cut = Display::WRAP_WIDTH_AI_REASON;
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
    auto projectResponse = projService_.getManagerProjects(currentUserId_);
    if (!projectResponse.success || projectResponse.data.empty())
    {
        showError("No projects found for your account.");
        ScreenUtils::readLine("Press Enter to continue");
        return std::nullopt;
    }

    std::cout << "\n================ MY PROJECTS ================\n";
    std::cout << std::left << std::setw(6) << "#" << std::setw(8) << "ID" << std::setw(25) << "Name" << "\n";
    std::cout << "───────────────────────────────────────────────\n";
    
    int row = 1;
    for (const auto& p : projectResponse.data)
    {
        std::cout << std::left << std::setw(6) << row++ << std::setw(8) << p.id 
                  << std::setw(25) << ScreenUtils::truncate(p.name, 24) << "\n";
    }
    std::cout << "───────────────────────────────────────────────\n";

    std::string projInput = ScreenUtils::readLine(promptText + " (or 0 to cancel)");
    projInput = ScreenUtils::trim(projInput);
    if (projInput.empty() || projInput == "0") return std::nullopt;

    int projectId = 0;
    
    auto parsedRow = ScreenUtils::safeParseInt(projInput);
    if (parsedRow && parsedRow.value() > 0 && parsedRow.value() <= (int)projectResponse.data.size())
    {
        projectId = projectResponse.data[parsedRow.value() - 1].id;
    }
    else
    {
        for (const auto& p : projectResponse.data)
        {
            if (std::to_string(p.id) == projInput || ScreenUtils::equalsIgnoreCase(p.name, projInput))
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

std::optional<AiCandidateDTO> AllocateResourceScreen::selectAICandidate(const std::vector<AiCandidateDTO>& candidates)
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
    if (selStr.empty()) return std::nullopt;
    auto parsedSel = ScreenUtils::safeParseInt(selStr);
    if (!parsedSel) 
    {
        showError(Messages::INVALID_SELECTION_FORMAT);
        return std::nullopt;
    }
    int selection = parsedSel.value();
    if (selection < 1 || selection > (int)candidates.size())
    {
        return std::nullopt;
    }

    return candidates[selection - 1];
}

std::optional<CreateAllocationRequest> AllocateResourceScreen::promptForAllocationRequest(
    int employeeId, int projectId)
{
    std::cout << "\n── Selected Employee ID: " << employeeId << " ─────────────────────────────\n";
    auto utilOpt = ScreenUtils::promptForIntBounds("Set Allocation Utilisation %", 1, 100, "Utilization must be between 1 and 100.");
    if (!utilOpt)
    {
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
        return std::nullopt;
    }
    int utilPercent = utilOpt.value();
    
    auto fromDateOpt = ScreenUtils::promptForDate("From Date", true);
    if (!fromDateOpt)
    {
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
        return std::nullopt;
    }
    std::string fromDate = fromDateOpt.value();

    auto toDateOpt = ScreenUtils::promptForDate("To Date", true);
    if (!toDateOpt)
    {
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
        return std::nullopt;
    }
    std::string toDate = toDateOpt.value();

    if (!toDate.empty() && toDate < fromDate)
    {
        showError("To Date cannot be before From Date.");
        ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
        return std::nullopt;
    }

    CreateAllocationRequest allocReq;
    allocReq.employeeId = employeeId;
    allocReq.projectId = projectId;
    allocReq.utilizationPercentage = utilPercent;
    allocReq.fromDate = fromDate;
    allocReq.toDate = toDate;

    return allocReq;
}

void AllocateResourceScreen::createAllocation(int empId, int projectId)
{
    auto requestOpt = promptForAllocationRequest(empId, projectId);
    if (!requestOpt)
    {
        return;
    }

    auto allocationResponse = allocService_.createAllocation(requestOpt.value());

    if (allocationResponse.success)
    {
        showSuccess("Allocation saved successfully! ✓");
    }
    else
    {
        showError(allocationResponse.message);
    }
    ConsoleInput::waitForEnter(Messages::PRESS_ENTER_TO_CONTINUE);
}

std::optional<int> AllocateResourceScreen::promptForEmployeeId()
{
    std::string empIdStr = ScreenUtils::readLine("Enter Employee ID");
    if (empIdStr.empty()) return std::nullopt;
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
    auto allocationResponse = allocService_.getProjectAllocations(projectId);
    

    if (!allocationResponse.success || allocationResponse.data.empty())
    {
        showInfo("No active allocations found on this project.");
        ScreenUtils::readLine(Messages::PRESS_ENTER_TO_CONTINUE);
        return std::nullopt;
    }
    
    auto activeAllocs = allocationResponse.data;
    auto allocData = fetchAllocationsWithNames(activeAllocs);
    displayActiveAllocations(allocData);

    return selectAllocation(activeAllocs);
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

std::optional<AllocationDTO> AllocateResourceScreen::selectAllocation(const std::vector<AllocationDTO>& activeAllocs)
{
    std::string selStr = ScreenUtils::readLine("Select resource to deallocate (enter #, or 0 to cancel)");
    if (selStr.empty()) return std::nullopt;
    auto parsedSel = ScreenUtils::safeParseInt(selStr);
    if (!parsedSel) 
    {
        showError(Messages::INVALID_SELECTION_FORMAT);
        return std::nullopt;
    }
    int selection = parsedSel.value();
    if (selection < 1 || selection > (int)activeAllocs.size())
    {
        return std::nullopt;
    }

    return activeAllocs[selection - 1];
}

void AllocateResourceScreen::endAllocation(int allocId)
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
    ScreenUtils::readLine(Messages::PRESS_ENTER_TO_CONTINUE);
}
