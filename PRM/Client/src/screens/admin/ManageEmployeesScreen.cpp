#include "admin/ManageEmployeesScreen.h"

#include <algorithm>
#include <cctype>
#include <iomanip>

#include "api/ApiException.h"
#include "dto/ApiResponse.h"
#include "dto/EmployeeDTO.h"
#include "screens/ScreenUtils.h"
#include "services/EmployeeClientService.h"
#include "utils/ConsoleInput.h"

using namespace AdminConstants;
using namespace AdminConstants::Employees;

ManageEmployeesScreen::ManageEmployeesScreen(EmployeeClientService& empService)
    : empService_(empService)
{
}

void ManageEmployeesScreen::displayMenu()
{
    decorator().render();
    std::cout << OPT_VIEW_ALL << ". View All Employees\n";
    std::cout << OPT_UPDATE << ". Update Employee\n";
    std::cout << OPT_DEACTIVATE << ". Deactivate Employee\n";
    std::cout << OPT_SKILLS << ". Manage Employee Skills\n";
    std::cout << OPT_ASSIGN_MGR << ". Assign Manager\n";
    std::cout << OPT_BACK << ". Back\n";
}

void ManageEmployeesScreen::show()
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        displayMenu();
        handleInput();
    }
}

void ManageEmployeesScreen::handleInput()
{
    std::string choice = ConsoleInput::readLine("Enter option");
    if (choice == OPT_VIEW_ALL)
    {
        viewAllEmployees();
    }
    else if (choice == OPT_UPDATE)
    {
        updateEmployee();
    }
    else if (choice == OPT_DEACTIVATE)
    {
        deactivateEmployee();
    }
    else if (choice == OPT_SKILLS)
    {
        manageEmployeeSkills();
    }
    else if (choice == OPT_ASSIGN_MGR)
    {
        assignManager();
    }
    else if (choice == OPT_BACK || ScreenUtils::equalsIgnoreCase(choice, "B"))
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1–6.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

void ManageEmployeesScreen::viewAllEmployees()
{
    try
    {
        auto response = empService_.viewAllEmployees();
        if (!response.success)
        {
            showError(response.message);
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        std::string filterStatus = "";
        std::string filterDept = "";

        while (true)
        {
            displayEmployees(response.data, filterStatus, filterDept);
            if (!promptFilterOptions(filterStatus, filterDept))
            {
                break;
            }
        }
    }
    catch (const std::exception& ex)
    {
        handleStandardExceptions(ex);
    }
}

void ManageEmployeesScreen::displayFilterMenu()
{
    std::cout << "\nFilter options:\n";
    std::cout << "1. Filter by Status only\n";
    std::cout << "2. Filter by Department only\n";
    std::cout << "3. Filter by both Status AND Department\n";
    std::cout << "4. Clear all filters\n";
}

std::optional<std::string> ManageEmployeesScreen::promptForStatusFilter()
{
    std::cout << "1. ALLOCATED   2. BENCH\n";
    std::string stVal = ConsoleInput::readLine("Status choice");
    if (stVal == "1") return STATUS_ALLOCATED;
    if (stVal == "2") return STATUS_BENCH;

    showError("Invalid status filter option.");
    ConsoleInput::waitForEnter("Press Enter to continue\n");
    return std::nullopt;
}

std::string ManageEmployeesScreen::promptForDepartmentFilter()
{
    return ConsoleInput::readLine("Department name");
}

void ManageEmployeesScreen::applyFilterChoice(const std::string& filterChoice,
                                              std::string& filterStatus, std::string& filterDept)
{
    if (filterChoice == "1")
    {
        auto stOpt = promptForStatusFilter();
        if (stOpt) filterStatus = stOpt.value();
        filterDept = "";
    }
    else if (filterChoice == "2")
    {
        filterStatus = "";
        filterDept = promptForDepartmentFilter();
    }
    else if (filterChoice == "3")
    {
        auto stOpt = promptForStatusFilter();
        if (stOpt) filterStatus = stOpt.value();
        filterDept = promptForDepartmentFilter();
    }
    else if (filterChoice == "4")
    {
        filterStatus = "";
        filterDept = "";
    }
    else
    {
        showError("Invalid filter option.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

bool ManageEmployeesScreen::promptFilterOptions(std::string& filterStatus, std::string& filterDept)
{
    std::cout << "[F] Filter by Status / Department     [B] Back\n";
    std::string choice = ConsoleInput::readLine("Enter choice");

    if (ScreenUtils::equalsIgnoreCase(choice, "B"))
    {
        return false;
    }

    if (ScreenUtils::equalsIgnoreCase(choice, "F"))
    {
        displayFilterMenu();
        std::string fOpt = ConsoleInput::readLine("Choice");
        applyFilterChoice(fOpt, filterStatus, filterDept);
    }
    return true;
}

void ManageEmployeesScreen::updateEmployee()
{
    try
    {
        auto empIdOpt = promptForId("Enter Employee ID");
        if (!empIdOpt) return;

        auto targetEmpOpt = fetchEmployeeById(empIdOpt.value());
        if (!targetEmpOpt) return;

        UpdateEmployeeRequest updateEmployeeRequest =
            promptForEmployeeUpdateRequest(targetEmpOpt.value());

        auto updateResponse = empService_.updateEmployee(empIdOpt.value(), updateEmployeeRequest);
        if (updateResponse.success)
        {
            showSuccess("Employee updated successfully. ✓");
        }
        else
        {
            showError(updateResponse.message);
        }
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const std::exception& ex)
    {
        handleStandardExceptions(ex);
    }
}

void ManageEmployeesScreen::promptForBasicDetails(const EmployeeDTO& targetEmp,
                                                  UpdateEmployeeRequest& req)
{
    std::string newName = ConsoleInput::readLine("Name (" + targetEmp.fullName + ")");
    std::string newEmail = ConsoleInput::readLine("Email (" + targetEmp.email + ")");
    std::string newDept = ConsoleInput::readLine("Department (" + targetEmp.department + ")");
    std::string newDesg = ConsoleInput::readLine("Designation (" + targetEmp.designation + ")");

    req.fullName = newName.empty() ? targetEmp.fullName : newName;
    req.email = newEmail.empty() ? targetEmp.email : newEmail;
    req.department = newDept.empty() ? targetEmp.department : newDept;
    req.designation = newDesg.empty() ? targetEmp.designation : newDesg;
}

void ManageEmployeesScreen::promptForEmployeeStatus(const EmployeeDTO& targetEmp,
                                                    UpdateEmployeeRequest& req)
{
    std::cout << "Status:\n";
    std::cout << "1. BENCH\n";
    std::cout << "2. ALLOCATED\n";
    std::string statusChoice = ConsoleInput::readLine("Choice");

    req.status = targetEmp.status;
    if (statusChoice == "1")
        req.status = STATUS_BENCH;
    else if (statusChoice == "2")
        req.status = STATUS_ALLOCATED;
}

UpdateEmployeeRequest ManageEmployeesScreen::promptForEmployeeUpdateRequest(
    const EmployeeDTO& targetEmp)
{
    std::cout << "\n── " << targetEmp.fullName << " ─────────────────────────────────\n";

    UpdateEmployeeRequest updateEmployeeRequest;
    promptForBasicDetails(targetEmp, updateEmployeeRequest);
    promptForEmployeeStatus(targetEmp, updateEmployeeRequest);
    updateEmployeeRequest.isActive = targetEmp.isActive;

    return updateEmployeeRequest;
}

void ManageEmployeesScreen::deactivateEmployee()
{
    try
    {
        auto empIdOpt = promptForId("Enter Employee ID");
        if (!empIdOpt) return;

        auto targetEmpOpt = fetchEmployeeById(empIdOpt.value());
        if (!targetEmpOpt) return;

        auto targetEmp = targetEmpOpt.value();
        if (!targetEmp.isActive)
        {
            showError("Employee is already deactivated.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }
        std::cout << "\n── " << targetEmp.fullName << " ─────────────────────────────────\n";
        std::cout << "Department: " << targetEmp.department << "\n";
        std::cout << "Status    : " << targetEmp.status << "\n\n";

        if (targetEmp.status == STATUS_ALLOCATED)
        {
            std::cout << "⚠ Warning: This employee's active allocations will end immediately.\n";
        }
        std::cout << "[D] Deactivate      [B] Back\n";
        std::string act = ConsoleInput::readLine("Enter choice");
        if (ScreenUtils::equalsIgnoreCase(act, "D"))
        {
            auto deactivateResponse = empService_.deactivateEmployee(empIdOpt.value());
            if (deactivateResponse.success)
            {
                showSuccess("Employee deactivated successfully. ✓");
            }
            else
            {
                showError(deactivateResponse.message);
            }
        }
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const std::exception& ex)
    {
        handleStandardExceptions(ex);
    }
}

void ManageEmployeesScreen::manageEmployeeSkills()
{
    try
    {
        auto empIdOpt = promptForId("Enter Employee ID");
        if (!empIdOpt) return;
        int empId = empIdOpt.value();

        while (true)
        {
            auto skillsRes = empService_.getEmployeeSkills(empId);
            if (!skillsRes.success)
            {
                showError(skillsRes.message);
                ConsoleInput::waitForEnter("Press Enter to continue\n");
                return;
            }

            auto skills = skillsRes.data;
            displayCurrentSkills(skills);

            std::cout << ADD_SKILL_OPTION << ". Add Skill\n";
            std::cout << UPDATE_SKILL_OPTION << ". Update Proficiency Level\n";
            std::cout << REMOVE_SKILL_OPTION << ". Remove Skill\n";
            std::cout << BACK_SKILL_OPTION << ". Back\n";
            std::string selectedOption = ConsoleInput::readLine("Enter option");

            if (!handleSkillOption(selectedOption, empId, skills))
            {
                break;
            }
        }
    }
    catch (const std::exception& ex)
    {
        handleStandardExceptions(ex);
    }
}

bool ManageEmployeesScreen::handleSkillOption(const std::string& selectedOption, int empId,
                                              const std::vector<SkillDTO>& skills)
{
    if (selectedOption == ADD_SKILL_OPTION)
    {
        executeAddSkill(empId);
    }
    else if (selectedOption == UPDATE_SKILL_OPTION)
    {
        executeUpdateSkill(empId, skills);
    }
    else if (selectedOption == REMOVE_SKILL_OPTION)
    {
        executeRemoveSkill(empId, skills);
    }
    else if (selectedOption == BACK_SKILL_OPTION ||
             ScreenUtils::equalsIgnoreCase(selectedOption, "B"))
    {
        return false;
    }
    else
    {
        showError("Invalid option.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    return true;
}

void ManageEmployeesScreen::displayCurrentSkills(const std::vector<SkillDTO>& skills)
{
    std::cout << "\n================ CURRENT SKILLS ================\n";
    int idx = 1;
    for (const auto& sk : skills)
    {
        std::cout << idx++ << ". " << sk.skillName << " (" << sk.category << ") — "
                  << sk.proficiency << "\n";
    }
    ScreenUtils::printDivider();
}

std::string ManageEmployeesScreen::promptForSkillCategory()
{
    std::cout << "Category:\n";
    std::cout << "1. Backend\n";
    std::cout << "2. Frontend\n";
    std::cout << "3. DevOps\n";
    std::cout << "4. QA\n";
    std::cout << "5. Other\n";
    std::string catChoice = ConsoleInput::readLine("Enter choice");
    if (catChoice == "1") return CATEGORY_BACKEND;
    if (catChoice == "2") return CATEGORY_FRONTEND;
    if (catChoice == "3") return CATEGORY_DEVOPS;
    if (catChoice == "4") return CATEGORY_QA;
    return CATEGORY_OTHER;
}

std::string ManageEmployeesScreen::promptForProficiency()
{
    std::cout << "Proficiency Level:\n";
    std::cout << "1. Beginner\n";
    std::cout << "2. Intermediate\n";
    std::cout << "3. Advanced\n";
    std::string profChoice = ConsoleInput::readLine("Enter choice");
    if (profChoice == "2") return PROFICIENCY_INTERMEDIATE;
    if (profChoice == "3") return PROFICIENCY_ADVANCED;
    return PROFICIENCY_BEGINNER;
}

AddSkillRequest ManageEmployeesScreen::promptForAddSkillRequest()
{
    AddSkillRequest addSkillRequest;
    addSkillRequest.skillName = ConsoleInput::readLine("Skill Name");
    addSkillRequest.category = promptForSkillCategory();
    addSkillRequest.proficiency = promptForProficiency();
    return addSkillRequest;
}

void ManageEmployeesScreen::executeAddSkill(int empId)
{
    AddSkillRequest addSkillRequest = promptForAddSkillRequest();
    auto addSkillResponse = empService_.addSkill(empId, addSkillRequest);
    if (addSkillResponse.success)
    {
        showSuccess("Skill added successfully");
    }
    else
    {
        showError(addSkillResponse.message);
    }
    ConsoleInput::waitForEnter("Press Enter to continue\n");
}

std::optional<int> ManageEmployeesScreen::promptForSkillIdFromList(
    const std::vector<SkillDTO>& skills, const std::string& prompt)
{
    auto itemNumOpt = promptForId(prompt);
    if (!itemNumOpt) return std::nullopt;
    int itemNum = itemNumOpt.value();

    if (itemNum < 1 || itemNum > (int)skills.size())
    {
        showError("Invalid skill number.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return std::nullopt;
    }
    return skills[itemNum - 1].skillId;
}

UpdateSkillRequest ManageEmployeesScreen::promptForUpdateSkillRequest()
{
    std::cout << "New Proficiency Level:\n";
    UpdateSkillRequest updateSkillRequest;
    updateSkillRequest.proficiency = promptForProficiency();
    return updateSkillRequest;
}

void ManageEmployeesScreen::executeUpdateSkill(int empId, const std::vector<SkillDTO>& skills)
{
    auto skillIdOpt = promptForSkillIdFromList(skills, "Enter Skill # to update");
    if (!skillIdOpt) return;

    UpdateSkillRequest updateSkillRequest = promptForUpdateSkillRequest();
    auto updateResponse = empService_.updateSkill(empId, skillIdOpt.value(), updateSkillRequest);
    if (updateResponse.success)
    {
        showSuccess("Skill updated successfully. ✓");
    }
    else
    {
        showError(updateResponse.message);
    }
    ConsoleInput::waitForEnter("Press Enter to continue\n");
}

void ManageEmployeesScreen::executeRemoveSkill(int empId, const std::vector<SkillDTO>& skills)
{
    auto skillIdOpt = promptForSkillIdFromList(skills, "Enter Skill # to remove");
    if (!skillIdOpt) return;

    auto removeSkillResponse = empService_.removeSkill(empId, skillIdOpt.value());
    if (removeSkillResponse.success)
    {
        showSuccess("Skill removed successfully. ✓");
    }
    else
    {
        showError(removeSkillResponse.message);
    }
    ConsoleInput::waitForEnter("Press Enter to continue\n");
}

void ManageEmployeesScreen::assignManager()
{
    try
    {
        auto employeeUserIdOpt = promptForId("Employee User ID");
        if (!employeeUserIdOpt) return;
        int employeeUserId = employeeUserIdOpt.value();

        auto managerUserIdOpt = promptForId("Manager User ID");
        if (!managerUserIdOpt) return;
        int managerUserId = managerUserIdOpt.value();

        if (employeeUserId == managerUserId)
        {
            showError("Employee and manager cannot be the same user.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        auto response = empService_.assignManager(employeeUserId, managerUserId);

        if (response.success)
        {
            showSuccess("Manager assigned successfully.");
        }
        else
        {
            showError(response.message);
        }
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

ScreenDecorator ManageEmployeesScreen::decorator() const
{
    return ScreenDecorator("MANAGE EMPLOYEES")
        .withWidth(DEFAULT_PANEL_WIDTH)
        .withPadding(DEFAULT_PADDING);
}

std::vector<EmployeeDTO> ManageEmployeesScreen::filterEmployees(
    const std::vector<EmployeeDTO>& employees, const std::string& filterStatus,
    const std::string& filterDept)
{
    std::vector<EmployeeDTO> filtered;
    for (const auto& emp : employees)
    {
        if (!emp.isActive) continue;

        if (!filterStatus.empty() && !ScreenUtils::equalsIgnoreCase(emp.status, filterStatus))
            continue;
        if (!filterDept.empty() && !ScreenUtils::equalsIgnoreCase(emp.department, filterDept))
            continue;

        filtered.push_back(emp);
    }
    return filtered;
}

void ManageEmployeesScreen::displayEmployeeTable(const std::vector<EmployeeDTO>& employees)
{
    std::cout << std::left << std::setw(ID_COLUMN_WIDTH) << "ID"
              << std::setw(EMPLOYEE_NAME_COLUMN_WIDTH) << "Name"
              << std::setw(DEPARTMENT_COLUMN_WIDTH) << "Department"
              << std::setw(STATUS_COLUMN_WIDTH) << "Status" << "\n";
    ScreenUtils::printDivider();

    for (const auto& emp : employees)
    {
        std::cout << std::left << std::setw(ID_COLUMN_WIDTH) << emp.id
                  << std::setw(EMPLOYEE_NAME_COLUMN_WIDTH)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(emp.fullName),
                                           EMPLOYEE_NAME_COLUMN_WIDTH - 1)
                  << std::setw(DEPARTMENT_COLUMN_WIDTH)
                  << ScreenUtils::truncate(ScreenUtils::valueOrDash(emp.department),
                                           DEPARTMENT_COLUMN_WIDTH - 1)
                  << std::setw(STATUS_COLUMN_WIDTH) << ScreenUtils::valueOrDash(emp.status) << "\n";
    }
    ScreenUtils::printDivider();
}

void ManageEmployeesScreen::displayEmployeeSummary(const std::vector<EmployeeDTO>& employees)
{
    int allocatedCount = 0;
    int benchCount = 0;
    int unknownCount = 0;
    for (const auto& emp : employees)
    {
        if (ScreenUtils::equalsIgnoreCase(emp.status, STATUS_ALLOCATED))
            allocatedCount++;
        else if (ScreenUtils::equalsIgnoreCase(emp.status, STATUS_BENCH))
            benchCount++;
        else
            unknownCount++;
    }
    std::cout << "Total: " << employees.size() << "  |  Allocated: " << allocatedCount
              << "  |  Bench: " << benchCount;
    if (unknownCount > 0)
    {
        std::cout << "  |  Unknown: " << unknownCount;
    }
    std::cout << "\n\n";
}

void ManageEmployeesScreen::displayEmployees(const std::vector<EmployeeDTO>& employees,
                                             const std::string& filterStatus,
                                             const std::string& filterDept)
{
    std::cout << "\n================ ALL EMPLOYEES ================\n";
    auto filtered = filterEmployees(employees, filterStatus, filterDept);
    displayEmployeeTable(filtered);
    displayEmployeeSummary(filtered);
}

std::optional<int> ManageEmployeesScreen::promptForId(const std::string& prompt)
{
    return ScreenUtils::promptForInt(prompt, "Invalid ID format.");
}

std::optional<EmployeeDTO> ManageEmployeesScreen::fetchEmployeeById(int empId)
{
    auto response = empService_.viewAllEmployees();
    if (!response.success)
    {
        showError("Failed to fetch employees list.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return std::nullopt;
    }

    for (const auto& emp : response.data)
    {
        if (emp.id == empId)
        {
            return emp;
        }
    }

    showError("Employee not found.");
    ConsoleInput::waitForEnter("Press Enter to continue\n");
    return std::nullopt;
}

void ManageEmployeesScreen::handleStandardExceptions(const std::exception& ex)
{
    if (dynamic_cast<const ApiException*>(&ex))
    {
        showError(ex.what());
    }
    else
    {
        showError("Something went wrong. Please try again.");
    }
    ConsoleInput::waitForEnter("Press Enter to continue\n");
}
