#include "admin/ManageProjectsScreen.h"

ManageProjectsScreen::ManageProjectsScreen()
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

void ManageProjectsScreen::show(ApiClient& apiClient)
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        displayMenu();
        handleInput(apiClient);
    }
}

void ManageProjectsScreen::handleInput(ApiClient& apiClient)
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        createProject(apiClient);
    }
    else if (choice == "2")
    {
        viewAllProjects(apiClient);
    }
    else if (choice == "3")
    {
        updateProjectDetails(apiClient);
    }
    else if (choice == "4")
    {
        manageMilestones(apiClient);
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

void ManageProjectsScreen::createProject(ApiClient& apiClient)
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
        int totalStoryPoints = totalSP.empty() ? 0 : std::stoi(totalSP);

        std::string mgrId = ScreenUtils::readLine("Assign Manager (Enter Manager User ID)");

        auto response = apiClient.post("/projects", {
            {"name", name},
            {"description", desc},
            {"start_date", startDate},
            {"end_date", endDate},
            {"status", status},
            {"total_story_points", totalStoryPoints},
            {"health_status", "ON_TRACK"},
            {"manager_id", std::stoi(mgrId)}
        });

        if (response["success"].get<bool>())
        {
            showSuccess("Project created successfully. ✓");
        }
        else
        {
            showError(response["message"].get<std::string>());
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to create project: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageProjectsScreen::viewAllProjects(ApiClient& apiClient)
{
    try
    {
        auto response = apiClient.get("/projects");
        if (!response["success"].get<bool>())
        {
            showError(response["message"].get<std::string>());
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto projects = response["data"];
        // clearScreen();
        std::cout << "\n================ ALL PROJECTS ================\n";
        std::cout << std::left << std::setw(6) << "ID"
                  << std::setw(20) << "Name"
                  << std::setw(15) << "End Date"
                  << std::setw(12) << "Status" << "\n";
        ScreenUtils::printDivider();

        for (const auto& proj : projects)
        {
            std::cout << std::left << std::setw(6) << proj["id"].get<int>()
                      << std::setw(20) << proj["name"].get<std::string>().substr(0, 19)
                      << std::setw(15) << proj["end_date"].get<std::string>()
                      << std::setw(12) << proj["status"].get<std::string>() << "\n";
        }
        ScreenUtils::printDivider();
        ScreenUtils::readLine("Press Enter to go back");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to retrieve projects: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageProjectsScreen::updateProjectDetails(ApiClient& apiClient)
{
    try
    {
        std::string projId = ScreenUtils::readLine("Enter Project ID");
        auto response = apiClient.get("/projects/" + projId);
        if (!response["success"].get<bool>())
        {
            showError("Project not found.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto targetProj = response["data"];
        // clearScreen();
        std::cout << "\n── " << targetProj["name"].get<std::string>() << " ─────────────────────────────────\n";
        std::string newName = ScreenUtils::readLine("Project Name (" + targetProj["name"].get<std::string>() + ")");
        std::string newDesc = ScreenUtils::readLine("Description (" + targetProj["description"].get<std::string>() + ")");
        std::string newStart = ScreenUtils::readLine("Start Date (" + targetProj["start_date"].get<std::string>() + ")");
        std::string newEnd = ScreenUtils::readLine("End Date (" + targetProj["end_date"].get<std::string>() + ")");

        std::cout << "Status:\n";
        std::cout << "1. PLANNED\n";
        std::cout << "2. ACTIVE\n";
        std::cout << "3. ON_HOLD\n";
        std::cout << "4. COMPLETED\n";
        std::string statusChoice = ScreenUtils::readLine("Choice");
        std::string newStatus = targetProj["status"].get<std::string>();
        if (statusChoice == "1") newStatus = "PLANNED";
        else if (statusChoice == "2") newStatus = "ACTIVE";
        else if (statusChoice == "3") newStatus = "ON_HOLD";
        else if (statusChoice == "4") newStatus = "COMPLETED";

        std::string newMgrId = ScreenUtils::readLine("Assign Manager (" + std::to_string(targetProj["manager_id"].get<int>()) + ")");

        if (newName.empty()) newName = targetProj["name"].get<std::string>();
        if (newDesc.empty()) newDesc = targetProj["description"].get<std::string>();
        if (newStart.empty()) newStart = targetProj["start_date"].get<std::string>();
        if (newEnd.empty()) newEnd = targetProj["end_date"].get<std::string>();
        int managerId = newMgrId.empty() ? targetProj["manager_id"].get<int>() : std::stoi(newMgrId);

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

        auto putRes = apiClient.put("/projects/" + projId, {
            {"name", newName},
            {"description", newDesc},
            {"start_date", newStart},
            {"end_date", newEnd},
            {"status", newStatus},
            {"total_story_points", targetProj.value("total_story_points", 0)},
            {"health_status", targetProj.value("health_status", "ON_TRACK")},
            {"manager_id", managerId}
        });

        if (putRes["success"].get<bool>())
        {
            showSuccess("Project details updated. ✓");
        }
        else
        {
            showError(putRes["message"].get<std::string>());
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Failed to update project: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageProjectsScreen::manageMilestones(ApiClient& apiClient)
{
    try
    {
        std::string projId = ScreenUtils::readLine("Enter Project ID");
        
        while (true)
        {
            auto msRes = apiClient.get("/projects/" + projId + "/milestones");
            if (!msRes["success"].get<bool>())
            {
                showError("Failed to fetch project milestones.");
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }

            auto milestones = msRes["data"];
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
                          << std::setw(20) << ms["title"].get<std::string>().substr(0, 19)
                          << std::setw(15) << ms["due_date"].get<std::string>()
                          << std::setw(12) << ms["status"].get<std::string>() << "\n";
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
                
                auto addRes = apiClient.post("/projects/" + projId + "/milestones", {
                    {"title", title},
                    {"due_date", dueDate},
                    {"story_points", 0},
                    {"status", "NOT_STARTED"},
                    {"health_flag", "NORMAL"}
                });

                if (addRes["success"].get<bool>())
                {
                    showSuccess("Milestone added successfully. ✓");
                }
                else
                {
                    showError(addRes["message"].get<std::string>());
                }
                ScreenUtils::readLine("Press Enter to continue");
            }
            else if (opt == "2")
            {
                int itemNum = std::stoi(ScreenUtils::readLine("Enter Milestone #"));
                if (itemNum < 1 || itemNum > (int)milestones.size())
                {
                    showError("Invalid milestone number.");
                    ScreenUtils::readLine("Press Enter to continue");
                    continue;
                }
                int milestoneId = milestones[itemNum - 1]["id"].get<int>();

                std::cout << "New Status:\n";
                std::cout << "1. NOT_STARTED\n";
                std::cout << "2. IN_PROGRESS\n";
                std::cout << "3. DONE\n";
                std::string stChoice = ScreenUtils::readLine("Choice");
                std::string status = "NOT_STARTED";
                if (stChoice == "2") status = "IN_PROGRESS";
                else if (stChoice == "3") status = "DONE";

                auto putRes = apiClient.put("/milestones/" + std::to_string(milestoneId) + "/status", {
                    {"status", status}
                });

                if (putRes["success"].get<bool>())
                {
                    showSuccess("Milestone updated. ✓");
                }
                else
                {
                    showError(putRes["message"].get<std::string>());
                }
                ScreenUtils::readLine("Press Enter to continue");
            }
            else if (opt == "3" || opt == "B" || opt == "b")
            {
                break;
            }
        }
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error managing milestones: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

ScreenDecorator ManageProjectsScreen::decorator() const
{
    return ScreenDecorator("MANAGE PROJECTS").withWidth(40).withPadding(2);
}
