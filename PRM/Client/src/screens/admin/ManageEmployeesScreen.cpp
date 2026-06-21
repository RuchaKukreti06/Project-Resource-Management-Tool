#include "admin/ManageEmployeesScreen.h"
#include "dto/ApiResponse.h"
#include "dto/EmployeeDTO.h"

ManageEmployeesScreen::ManageEmployeesScreen()
{
}

void ManageEmployeesScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    std::cout << "1. View All Employees\n";
    std::cout << "2. Update Employee\n";
    std::cout << "3. Deactivate Employee\n";
    std::cout << "4. Manage Employee Skills\n";
    std::cout << "5. Assign Manager\n";
    std::cout << "6. Back\n";
}

void ManageEmployeesScreen::show(ApiClient& apiClient)
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        displayMenu();
        handleInput(apiClient);
    }
}

void ManageEmployeesScreen::handleInput(ApiClient& apiClient)
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        viewAllEmployees(apiClient);
    }
    else if (choice == "2")
    {
        updateEmployee(apiClient);
    }
    else if (choice == "3")
    {
        deactivateEmployee(apiClient);
    }
    else if (choice == "4")
    {
        manageEmployeeSkills(apiClient);
    }
    else if (choice == "5")
    {
        assignManager(apiClient);
    }
    else if (choice == "6" || choice == "B" || choice == "b")
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1–6.");
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageEmployeesScreen::viewAllEmployees(ApiClient& apiClient)
{
    try
    {
        auto response = ApiListResponse<EmployeeDTO>::fromJson(apiClient.get("/employees"));
        if (!response.success)
        {
            showError(response.message);
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto employees = response.data;
        
        std::string filterStatus = "";
        std::string filterDept = "";

        while (true)
        {
            // clearScreen();
            std::cout << "\n================ ALL EMPLOYEES ================\n";
            std::cout << std::left << std::setw(6) << "ID"
                      << std::setw(20) << "Name"
                      << std::setw(15) << "Department"
                      << std::setw(12) << "Status" << "\n";
            ScreenUtils::printDivider();

            int allocatedCount = 0;
            int benchCount = 0;
            int totalShown = 0;

            for (const auto& emp : employees)
            {
                if (!emp.isActive) continue;

                std::string status = emp.status;
                std::string dept = emp.department;

                if (!filterStatus.empty() && status != filterStatus) continue;
                if (!filterDept.empty() && dept != filterDept) continue;

                std::cout << std::left << std::setw(6) << emp.id
                          << std::setw(20) << emp.fullName
                          << std::setw(15) << dept
                          << std::setw(12) << status << "\n";
                
                if (status == "ALLOCATED") allocatedCount++;
                else benchCount++;
                totalShown++;
            }
            ScreenUtils::printDivider();
            std::cout << "Total: " << totalShown << "  |  Allocated: " << allocatedCount << "  |  Bench: " << benchCount << "\n\n";

            std::cout << "[F] Filter by Status / Department     [B] Back\n";
            std::string choice = ScreenUtils::readLine("Enter choice");
            if (choice == "F" || choice == "f")
            {
                std::cout << "\nFilter options:\n";
                std::cout << "1. Filter by Status only\n";
                std::cout << "2. Filter by Department only\n";
                std::cout << "3. Filter by both Status AND Department\n";
                std::cout << "4. Clear all filters\n";
                std::string fOpt = ScreenUtils::readLine("Choice");
                if (fOpt == "1")
                {
                    std::cout << "1. ALLOCATED   2. BENCH\n";
                    std::string stVal = ScreenUtils::readLine("Status choice");
                    if (stVal == "1") filterStatus = "ALLOCATED";
                    else if (stVal == "2") filterStatus = "BENCH";
                    filterDept = "";
                }
                else if (fOpt == "2")
                {
                    filterStatus = "";
                    filterDept = ScreenUtils::readLine("Department name");
                }
                else if (fOpt == "3")
                {
                    std::cout << "1. ALLOCATED   2. BENCH\n";
                    std::string stVal = ScreenUtils::readLine("Status choice");
                    if (stVal == "1") filterStatus = "ALLOCATED";
                    else if (stVal == "2") filterStatus = "BENCH";
                    filterDept = ScreenUtils::readLine("Department name");
                }
                else if (fOpt == "4")
                {
                    filterStatus = "";
                    filterDept = "";
                }
            }
            else if (choice == "B" || choice == "b")
            {
                break;
            }
        }
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to view employees: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageEmployeesScreen::updateEmployee(ApiClient& apiClient)
{
    try
    {
        std::string empId = ScreenUtils::readLine("Enter Employee ID");
        auto response = ApiListResponse<EmployeeDTO>::fromJson(apiClient.get("/employees"));
        if (!response.success)
        {
            showError("Failed to fetch employees list.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        EmployeeDTO targetEmp;
        bool found = false;
        for (const auto& emp : response.data)
        {
            if (std::to_string(emp.id) == empId)
            {
                targetEmp = emp;
                found = true;
                break;
            }
        }

        if (!found)
        {
            showError("Employee not found.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        // clearScreen();
        std::cout << "\n── " << targetEmp.fullName << " ─────────────────────────────────\n";
        std::string newName = ScreenUtils::readLine("Name (" + targetEmp.fullName + ")");
        std::string newEmail = ScreenUtils::readLine("Email (" + targetEmp.email + ")");
        std::string newDept = ScreenUtils::readLine("Department (" + targetEmp.department + ")");
        std::string newDesg = ScreenUtils::readLine("Designation (" + targetEmp.designation + ")");

        std::cout << "Status:\n";
        std::cout << "1. BENCH\n";
        std::cout << "2. ALLOCATED\n";
        std::string statusChoice = ScreenUtils::readLine("Choice");
        std::string newStatus = targetEmp.status;
        if (statusChoice == "1") newStatus = "BENCH";
        else if (statusChoice == "2") newStatus = "ALLOCATED";

        if (newName.empty()) newName = targetEmp.fullName;
        if (newEmail.empty()) newEmail = targetEmp.email;
        if (newDept.empty()) newDept = targetEmp.department;
        if (newDesg.empty()) newDesg = targetEmp.designation;

        auto putRes = ApiEmptyResponse::fromJson(apiClient.put("/employees/" + empId, {
            {"full_name", newName},
            {"email", newEmail},
            {"department", newDept},
            {"designation", newDesg},
            {"status", newStatus},
            {"is_active", targetEmp.isActive}
        }));

        if (putRes.success)
        {
            showSuccess("Employee updated successfully. ✓");
        }
        else
        {
            showError(putRes.message);
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to update employee: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageEmployeesScreen::deactivateEmployee(ApiClient& apiClient)
{
    try
    {
        std::string empId = ScreenUtils::readLine("Enter Employee ID");
        auto response = ApiListResponse<EmployeeDTO>::fromJson(apiClient.get("/employees"));
        if (!response.success)
        {
            showError("Failed to fetch employees list.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        EmployeeDTO targetEmp;
        bool found = false;
        for (const auto& emp : response.data)
        {
            if (std::to_string(emp.id) == empId)
            {
                targetEmp = emp;
                found = true;
                break;
            }
        }

        if (!found)
        {
            showError("Employee not found.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        // clearScreen();
        std::cout << "\n── " << targetEmp.fullName << " ─────────────────────────────────\n";
        std::cout << "Department: " << targetEmp.department << "\n";
        std::cout << "Status    : " << targetEmp.status << "\n\n";

        std::cout << "⚠ Warning: This employee's active allocations will end immediately.\n";
        std::cout << "Are you sure you want to deactivate? (Y/N): ";
        std::string confirm = ScreenUtils::readLine("Choice");
        if (confirm == "Y" || confirm == "y")
        {
            auto patchRes = ApiEmptyResponse::fromJson(apiClient.patch("/employees/" + empId + "/deactivate", {}));
            if (patchRes.success)
            {
                showSuccess("Employee deactivated. ✓");
            }
            else
            {
                showError(patchRes.message);
            }
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to deactivate employee: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageEmployeesScreen::manageEmployeeSkills(ApiClient& apiClient)
{
    try
    {
        std::string empId = ScreenUtils::readLine("Enter Employee ID");
        
        while (true)
        {
            auto skillsRes = ApiListResponse<SkillDTO>::fromJson(apiClient.get("/employees/" + empId + "/skills"));
            if (!skillsRes.success)
            {
                showError(skillsRes.message);
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }

            // clearScreen();
            std::cout << "\n================ CURRENT SKILLS ================\n";
            auto skills = skillsRes.data;
            int idx = 1;
            for (const auto& sk : skills)
            {
                std::cout << idx++ << ". " << sk.skillName
                          << " (" << sk.category << ") — "
                          << sk.proficiency << "\n";
            }
            ScreenUtils::printDivider();

            std::cout << "1. Add Skill\n";
            std::cout << "2. Update Proficiency Level\n";
            std::cout << "3. Remove Skill\n";
            std::cout << "4. Back\n";
            std::string opt = ScreenUtils::readLine("Enter option");

            if (opt == "1")
            {
                std::string skillName = ScreenUtils::readLine("Skill Name");
                std::cout << "Category:\n";
                std::cout << "1. Backend\n";
                std::cout << "2. Frontend\n";
                std::cout << "3. DevOps\n";
                std::cout << "4. QA\n";
                std::cout << "5. Other\n";
                std::string catChoice = ScreenUtils::readLine("Enter choice");
                std::string category = "OTHER";
                if (catChoice == "1") category = "BACKEND";
                else if (catChoice == "2") category = "FRONTEND";
                else if (catChoice == "3") category = "DEVOPS";
                else if (catChoice == "4") category = "QA";

                std::cout << "Proficiency Level:\n";
                std::cout << "1. Beginner\n";
                std::cout << "2. Intermediate\n";
                std::cout << "3. Advanced\n";
                std::string profChoice = ScreenUtils::readLine("Enter choice");
                std::string proficiency = "BEGINNER";
                if (profChoice == "2") proficiency = "INTERMEDIATE";
                else if (profChoice == "3") proficiency = "ADVANCED";

                auto addRes = ApiEmptyResponse::fromJson(apiClient.post("/employees/" + empId + "/skills", {
                    {"skill_name", skillName},
                    {"category", category},
                    {"proficiency", proficiency}
                }));

                if (addRes.success)
                {
                    showSuccess("Skill added successfully. ✓");
                }
                else
                {
                    showError(addRes.message);
                }
                ScreenUtils::readLine("Press Enter to continue");
            }
            else if (opt == "2")
            {
                std::string inputStr = ScreenUtils::readLine("Enter Skill # to update");
                auto parsedInput = ScreenUtils::safeParseInt(inputStr);
                if (!parsedInput) throw std::invalid_argument("Invalid skill number format");
                int itemNum = parsedInput.value();
                if (itemNum < 1 || itemNum > (int)skills.size())
                {
                    showError("Invalid skill number.");
                    ScreenUtils::readLine("Press Enter to continue");
                    continue;
                }
                int skillId = skills[itemNum - 1].skillId;

                std::cout << "New Proficiency Level:\n";
                std::cout << "1. Beginner\n";
                std::cout << "2. Intermediate\n";
                std::cout << "3. Advanced\n";
                std::string profChoice = ScreenUtils::readLine("Enter choice");
                std::string proficiency = "BEGINNER";
                if (profChoice == "2") proficiency = "INTERMEDIATE";
                else if (profChoice == "3") proficiency = "ADVANCED";

                auto putRes = ApiEmptyResponse::fromJson(apiClient.put("/employees/" + empId + "/skills/" + std::to_string(skillId), {
                    {"proficiency", proficiency}
                }));

                if (putRes.success)
                {
                    showSuccess("Skill updated successfully. ✓");
                }
                else
                {
                    showError(putRes.message);
                }
                ScreenUtils::readLine("Press Enter to continue");
            }
            else if (opt == "3")
            {
                std::string inputStr = ScreenUtils::readLine("Enter Skill # to remove");
                auto parsedInput = ScreenUtils::safeParseInt(inputStr);
                if (!parsedInput) throw std::invalid_argument("Invalid skill number format");
                int itemNum = parsedInput.value();
                if (itemNum < 1 || itemNum > (int)skills.size())
                {
                    showError("Invalid skill number.");
                    ScreenUtils::readLine("Press Enter to continue");
                    continue;
                }
                int skillId = skills[itemNum - 1].skillId;

                auto delRes = ApiEmptyResponse::fromJson(apiClient.del("/employees/" + empId + "/skills/" + std::to_string(skillId)));
                if (delRes.success)
                {
                    showSuccess("Skill removed. ✓");
                }
                else
                {
                    showError(delRes.message);
                }
                ScreenUtils::readLine("Press Enter to continue");
            }
            else if (opt == "4" || opt == "B" || opt == "b")
            {
                break;
            }
        }
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error managing skills: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageEmployeesScreen::assignManager(ApiClient& apiClient)
{
    try
    {
        std::string empUserId = ScreenUtils::readLine("Employee User ID");
        std::string mgrUserId = ScreenUtils::readLine("Manager User ID");
        auto parsedMgrId = ScreenUtils::safeParseInt(mgrUserId);
        if (!parsedMgrId) throw std::invalid_argument("Invalid manager ID format");
        int mgrIdParsed = parsedMgrId.value();

        auto res = ApiEmptyResponse::fromJson(apiClient.put("/users/" + empUserId + "/assign-manager",
                                 {{"manager_id", mgrIdParsed}}));

        if (res.success)
        {
            showSuccess("Manager assigned successfully.");
        }
        else
        {
            showError(res.message);
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to assign manager: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}


ScreenDecorator ManageEmployeesScreen::decorator() const
{
    return ScreenDecorator("MANAGE EMPLOYEES").withWidth(40).withPadding(2);
}
