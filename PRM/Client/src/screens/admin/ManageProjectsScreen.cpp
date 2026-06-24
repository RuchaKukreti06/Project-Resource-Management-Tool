#include "admin/ManageProjectsScreen.h"
#include "dto/ApiResponse.h"
#include "dto/ProjectDTO.h"
#include <iomanip>
#include <iostream>
#include "api/ApiException.h"
#include "services/ProjectClientService.h"
ManageProjectsScreen::ManageProjectsScreen(ProjectClientService& projService)
    : projService_(projService)
{
}

void ManageProjectsScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    std::cout << "1. Create Project\n";
    std::cout << "2. View All Projects\n";
    std::cout << "3. Update Project Details\n";
    std::cout << "4. Manage Milestones\n";
    std::cout << "5. Back\n";
}

void ManageProjectsScreen::show()
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        displayMenu();
        handleInput();
    }
}

void ManageProjectsScreen::handleInput()
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        createProject();
    }
    else if (choice == "2")
    {
        viewAllProjects();
    }
    else if (choice == "3")
    {
        updateProjectDetails();
    }
    else if (choice == "4")
    {
        manageMilestones();
    }
    else if (choice == "5" || choice == "B" || choice == "b")
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1–5.");
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageProjectsScreen::createProject()
{
    try
    {
        std::cout << "\n========== CREATE PROJECT ==========\n";
        std::string name = ScreenUtils::readLine("Project Name");

        // Validate project name — no backslashes or control characters
        for (unsigned char c : name) {
            if (c < 0x20 || c == 0x7F || c == '\\') {
                showError("Project name contains invalid characters (no backslashes or control chars allowed).");
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }
        }

        std::string desc = ScreenUtils::readLine("Description");
        std::string startDate = ScreenUtils::readLine("Start Date (YYYY-MM-DD)");
        std::string endDate = ScreenUtils::readLine("End Date (YYYY-MM-DD)");

        // Validate date logic
        if (!startDate.empty() && !endDate.empty() && endDate <= startDate) {
            showError("End Date must be strictly after Start Date.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }
        
        std::cout << "Status:\n";
        std::cout << "1. PLANNED\n";
        std::cout << "2. ACTIVE\n";
        std::cout << "3. ON_HOLD\n";
        std::string statusChoice = ScreenUtils::readLine("Choice");
        std::string status = "PLANNED";
        if (statusChoice == "2") status = "ACTIVE";
        else if (statusChoice == "3") status = "ON_HOLD";

        std::string totalSP = ScreenUtils::readLine("Total Story Points (leave blank for 0)");
        auto parsedSP = ScreenUtils::safeParseInt(totalSP);
        int totalStoryPoints = totalSP.empty() ? 0 : (parsedSP ? parsedSP.value() : throw std::invalid_argument("Invalid story points format"));

        std::string mgrId = ScreenUtils::readLine("Assign Manager (Enter Manager User ID)");
        auto parsedMgrId = ScreenUtils::safeParseInt(mgrId);
        if (!parsedMgrId) throw std::invalid_argument("Invalid manager ID format");
        int mgrIdParsed = parsedMgrId.value();

        CreateProjectRequest req;
        req.name = name;
        req.description = desc;
        req.startDate = startDate;
        req.endDate = endDate;
        req.status = status;
        req.totalStoryPoints = totalStoryPoints;
        req.healthStatus = "ON_TRACK";
        req.managerId = mgrIdParsed;

        auto postRes = projService_.createProject(req);

        if (postRes.success)
        {
            showSuccess("Project created successfully. ✓");
        }
        else
        {
            showError(postRes.message);
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

void ManageProjectsScreen::viewAllProjects()
{
    try
    {
        auto response = projService_.viewAllProjects();
        if (!response.success)
        {
            showError(response.message);
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto projects = response.data;
        // clearScreen();
        std::cout << "\n================ ALL PROJECTS ================\n";
        std::cout << std::left << std::setw(6) << "ID"
                  << std::setw(20) << "Name"
                  << std::setw(15) << "End Date"
                  << std::setw(12) << "Status" << "\n";
        ScreenUtils::printDivider();

        for (const auto& proj : projects)
        {
            std::cout << std::left << std::setw(6) << proj.id
                      << std::setw(20) << proj.name.substr(0, 19)
                      << std::setw(15) << proj.endDate
                      << std::setw(12) << proj.status << "\n";
        }
        ScreenUtils::printDivider();
        ScreenUtils::readLine("Press Enter to go back");
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

void ManageProjectsScreen::updateProjectDetails()
{
    try
    {
        std::string projId = ScreenUtils::readLine("Enter Project ID");
        auto response = projService_.getProject(std::stoi(projId));
        if (!response.success || !response.data)
        {
            showError("Project not found.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto targetProj = *response.data;
        // clearScreen();
        std::cout << "\n── " << targetProj.name << " ─────────────────────────────────\n";
        std::string newName = ScreenUtils::readLine("Project Name (" + targetProj.name + ")");
        std::string newDesc = ScreenUtils::readLine("Description (" + targetProj.description + ")");
        std::string newStart = ScreenUtils::readLine("Start Date (" + targetProj.startDate + ")");
        std::string newEnd = ScreenUtils::readLine("End Date (" + targetProj.endDate + ")");

        std::cout << "Status:\n";
        std::cout << "1. PLANNED\n";
        std::cout << "2. ACTIVE\n";
        std::cout << "3. ON_HOLD\n";
        std::cout << "4. COMPLETED\n";
        std::string statusChoice = ScreenUtils::readLine("Choice");
        std::string newStatus = targetProj.status;
        if (statusChoice == "1") newStatus = "PLANNED";
        else if (statusChoice == "2") newStatus = "ACTIVE";
        else if (statusChoice == "3") newStatus = "ON_HOLD";
        else if (statusChoice == "4") newStatus = "COMPLETED";

        std::string newMgrId = ScreenUtils::readLine("Assign Manager (" + std::to_string(targetProj.managerId) + ")");

        if (newName.empty()) newName = targetProj.name;
        if (newDesc.empty()) newDesc = targetProj.description;
        if (newStart.empty()) newStart = targetProj.startDate;
        if (newEnd.empty()) newEnd = targetProj.endDate;
        int managerId = newMgrId.empty() ? targetProj.managerId : ScreenUtils::safeParseInt(newMgrId).value_or(targetProj.managerId);

        // Validate project name — no backslashes or control characters
        for (unsigned char c : newName) {
            if (c < 0x20 || c == 0x7F || c == '\\') {
                showError("Project name contains invalid characters.");
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }
        }

        // Validate date logic
        if (!newStart.empty() && !newEnd.empty() && newEnd <= newStart) {
            showError("End Date must be strictly after Start Date.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        CreateProjectRequest req;
        req.name = newName;
        req.description = newDesc;
        req.startDate = newStart;
        req.endDate = newEnd;
        req.status = newStatus;
        req.totalStoryPoints = targetProj.totalStoryPoints;
        req.healthStatus = targetProj.healthStatus;
        req.managerId = managerId;

        auto putRes = projService_.updateProject(targetProj.id, req);

        if (putRes.success)
        {
            showSuccess("Project details updated. ✓");
        }
        else
        {
            showError(putRes.message);
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

void ManageProjectsScreen::manageMilestones()
{
    try
    {
        std::string projId = ScreenUtils::readLine("Enter Project ID");
        int pId = std::stoi(projId);
        
        while (true)
        {
            auto msRes = projService_.getProjectMilestones(pId);
            if (!msRes.success)
            {
                showError("Failed to fetch project milestones.");
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }

            auto milestones = msRes.data;
            // clearScreen();
            std::cout << "\n================ MILESTONES ================\n";
            std::cout << std::left << std::setw(6) << "#"
                      << std::setw(20) << "Title"
                      << std::setw(15) << "Due Date"
                      << std::setw(12) << "Status" << "\n";
            ScreenUtils::printDivider();

            int idx = 1;
            for (const auto& ms : milestones)
            {
                std::cout << std::left << std::setw(6) << idx++
                          << std::setw(20) << ms.title.substr(0, 19)
                          << std::setw(15) << ms.dueDate
                          << std::setw(12) << ms.status << "\n";
            }
            ScreenUtils::printDivider();

            std::cout << "1. Add Milestone\n";
            std::cout << "2. Update Milestone Status\n";
            std::cout << "3. Back\n";
            std::string opt = ScreenUtils::readLine("Enter option");

            if (opt == "1")
            {
                std::string title = ScreenUtils::readLine("Milestone Title");
                std::string dueDate = ScreenUtils::readLine("Due Date (YYYY-MM-DD)");
                
                AddMilestoneRequest req;
                req.title = title;
                req.dueDate = dueDate;
                req.storyPoints = 0;
                req.status = "NOT_STARTED";
                req.healthFlag = "NORMAL";

                auto postRes = projService_.addMilestone(pId, req);

                if (postRes.success)
                {
                    showSuccess("Milestone added successfully. ✓");
                }
                else
                {
                    showError(postRes.message);
                }
                ScreenUtils::readLine("Press Enter to continue");
            }
            else if (opt == "2")
            {
                std::string inputStr = ScreenUtils::readLine("Enter Milestone #");
                auto parsedInput = ScreenUtils::safeParseInt(inputStr);
                if (!parsedInput) throw std::invalid_argument("Invalid milestone number");
                int itemNum = parsedInput.value();
                if (itemNum < 1 || itemNum > (int)milestones.size())
                {
                    showError("Invalid milestone number.");
                    ScreenUtils::readLine("Press Enter to continue");
                    continue;
                }
                int milestoneId = milestones[itemNum - 1].id;

                std::cout << "New Status:\n";
                std::cout << "1. NOT_STARTED\n";
                std::cout << "2. IN_PROGRESS\n";
                std::cout << "3. DONE\n";
                std::string stChoice = ScreenUtils::readLine("Choice");
                std::string status = "NOT_STARTED";
                if (stChoice == "2") status = "IN_PROGRESS";
                else if (stChoice == "3") status = "DONE";

                auto putRes = projService_.updateMilestoneStatus(milestoneId, status);

                if (putRes.success)
                {
                    showSuccess("Milestone updated. ✓");
                }
                else
                {
                    showError(putRes.message);
                }
                ScreenUtils::readLine("Press Enter to continue");
            }
            else if (opt == "3" || opt == "B" || opt == "b")
            {
                break;
            }
        }
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

ScreenDecorator ManageProjectsScreen::decorator() const
{
    return ScreenDecorator("MANAGE PROJECTS").withWidth(40).withPadding(2);
}
