#include "manager/AllocateResourceScreen.h"
#include "AuthSession.h"
#include <ctime>

AllocateResourceScreen::AllocateResourceScreen()
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

void AllocateResourceScreen::show(ApiClient& apiClient)
{
    while (true)
    {
        displayMenu();
        std::string choice = ScreenUtils::readLine("Enter option");
        if (choice == "1")
        {
            findResourceAI(apiClient);
        }
        else if (choice == "2")
        {
            allocateDirectly(apiClient);
        }
        else if (choice == "3")
        {
            endAllocation(apiClient);
        }
        else if (choice == "4" || choice == "B" || choice == "b")
        {
            break;
        }
        else
        {
            showError("Invalid option. Please enter 1–4.");
            ScreenUtils::readLine("Press Enter to continue");
        }
    }
}

void AllocateResourceScreen::handleInput(ApiClient& apiClient)
{
}

void AllocateResourceScreen::findResourceAI(ApiClient& apiClient)
{
    try
    {
        std::cout << "\nStep 1 — Select Project\n";
        std::string projectInput = ScreenUtils::readLine("Enter project name or ID");

        std::cout << "\nStep 2 — Describe your requirement\n";
        std::string reqText = ScreenUtils::readLine("Type what kind of resource you need");

        std::cout << "\nSearching... (AI matching in progress)\n";
        
        nlohmann::json payload = {{"requirement", reqText}};
        auto response = apiClient.post("/ai/skill-match", payload);

        if (!response.contains("success") || !response["success"].get<bool>())
        {
            showError("AI service error. Please check the API key in System Configuration.");
            return;
        }

        auto data = response["data"];

        // Normalise: if LLM returned a JSON string in "raw", try to parse it as array
        std::vector<nlohmann::json> candidates;
        nlohmann::json parsedData = nlohmann::json::array();
        if (data.is_array())
        {
            parsedData = data;
        }
        else if (data.contains("raw"))
        {
            try { parsedData = nlohmann::json::parse(data["raw"].get<std::string>()); }
            catch (...) {}
        }
        if (parsedData.is_array())
            for (const auto& item : parsedData) candidates.push_back(item);

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
                std::string reason = item.value("reason", "");

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
                          << std::setw(W_ID)   << item.value("employee_id", 0)
                          << std::setw(W_NAME) << item.value("name", "Unknown").substr(0, W_NAME - 1)
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
        int selection = std::stoi(selStr);
        if (selection < 1 || selection > (int)candidates.size())
        {
            return;
        }

        auto selectedEmp = candidates[selection - 1];
        int empId = selectedEmp["employee_id"].get<int>();

        std::cout << "\n── " << selectedEmp.value("name", "Unknown") << " ─────────────────────────────────\n";
        std::cout << "Current Utilisation: 0%   (fully on bench)\n\n";

        std::string utilStr = ScreenUtils::readLine("Set Allocation Utilisation %");
        std::string fromDate = ScreenUtils::readLine("From Date (YYYY-MM-DD)");
        std::string toDate = ScreenUtils::readLine("To Date (YYYY-MM-DD)");

        // Find Project ID
        int projectId = 0;
        auto projRes = apiClient.get("/projects");
        if (projRes["success"].get<bool>())
        {
            for (const auto& p : projRes["data"])
            {
                if (std::to_string(p["id"].get<int>()) == projectInput || p["name"].get<std::string>() == projectInput)
                {
                    projectId = p["id"].get<int>();
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

        auto allocRes = apiClient.post("/allocations", {
            {"employee_id", empId},
            {"project_id", projectId},
            {"utilization_percentage", std::stoi(utilStr)},
            {"from_date", fromDate},
            {"to_date", toDate}
        });

        if (allocRes["success"].get<bool>())
        {
            showSuccess("Allocation saved successfully! ✓");
        }
        else
        {
            showError(allocRes["message"].get<std::string>());
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error during AI allocation: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void AllocateResourceScreen::allocateDirectly(ApiClient& apiClient)
{
    try
    {
        std::string projInput = ScreenUtils::readLine("Select Project (Enter name or ID)");
        std::string empId = ScreenUtils::readLine("Enter Employee ID");

        // Find Project ID
        int projectId = 0;
        auto projRes = apiClient.get("/projects");
        if (projRes["success"].get<bool>())
        {
            for (const auto& p : projRes["data"])
            {
                if (std::to_string(p["id"].get<int>()) == projInput || p["name"].get<std::string>() == projInput)
                {
                    projectId = p["id"].get<int>();
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
        std::string fromDate = ScreenUtils::readLine("From Date (YYYY-MM-DD)");
        std::string toDate = ScreenUtils::readLine("To Date (YYYY-MM-DD)");

        auto allocRes = apiClient.post("/allocations", {
            {"employee_id", std::stoi(empId)},
            {"project_id", projectId},
            {"utilization_percentage", std::stoi(utilStr)},
            {"from_date", fromDate},
            {"to_date", toDate}
        });

        if (allocRes["success"].get<bool>())
        {
            showSuccess("Allocation saved successfully. ✓");
        }
        else
        {
            showError(allocRes["message"].get<std::string>());
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error during direct allocation: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void AllocateResourceScreen::endAllocation(ApiClient& apiClient)
{
    try
    {
        std::string projInput = ScreenUtils::readLine("Select Project (Enter name or ID)");

        int projectId = 0;
        auto projRes = apiClient.get("/projects");
        if (projRes["success"].get<bool>())
        {
            for (const auto& p : projRes["data"])
            {
                if (std::to_string(p["id"].get<int>()) == projInput || p["name"].get<std::string>() == projInput)
                {
                    projectId = p["id"].get<int>();
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

        // Fetch allocations
        auto allocRes = apiClient.get("/projects/" + std::to_string(projectId) + "/allocations");
        
        // clearScreen();
        std::cout << "\nActive Allocations on this project:\n";
        std::cout << std::left << std::setw(6) << "#"
                  << std::setw(20) << "Employee"
                  << std::setw(8) << "%"
                  << std::setw(12) << "From"
                  << std::setw(12) << "To" << "\n";
        ScreenUtils::printDivider();

        std::vector<nlohmann::json> activeAllocs;
        int idx = 1;
        if (!allocRes.contains("data") || !allocRes["data"].is_array())
        {
            showInfo("No active allocations found on this project.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }
        for (const auto& alloc : allocRes["data"])
        {
            activeAllocs.push_back(alloc);
            int empId = alloc["employee_id"].get<int>();
            
            // fetch emp name
            std::string empName = "Emp " + std::to_string(empId);
            auto empRes = apiClient.get("/employees");
            if (empRes.contains("data"))
            {
                for (const auto& e : empRes["data"])
                {
                    if (e["id"].get<int>() == empId)
                    {
                        empName = e["full_name"].get<std::string>();
                        break;
                    }
                }
            }

            std::cout << std::left << std::setw(6) << idx++
                      << std::setw(20) << empName.substr(0, 19)
                      << std::setw(8) << (std::to_string(alloc["utilization_percentage"].get<int>()) + "%")
                      << std::setw(12) << alloc["from_date"].get<std::string>()
                      << std::setw(12) << alloc["to_date"].get<std::string>() << "\n";
        }
        ScreenUtils::printDivider();

        if (activeAllocs.empty())
        {
            showInfo("No active allocations found on this project.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        std::string selStr = ScreenUtils::readLine("Select allocation to end");
        int selection = std::stoi(selStr);
        if (selection < 1 || selection > (int)activeAllocs.size())
        {
            return;
        }

        auto targetAlloc = activeAllocs[selection - 1];
        int allocId = targetAlloc["id"].get<int>();

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

            auto response = apiClient.put("/allocations/" + std::to_string(allocId) + "/end", {
                {"end_date", std::string(buffer)}
            });

            if (response["success"].get<bool>())
            {
                showSuccess("Allocation ended successfully. ✓");
            }
            else
            {
                showError(response["message"].get<std::string>());
            }
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error ending allocation: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

ScreenDecorator AllocateResourceScreen::decorator() const
{
    return ScreenDecorator("ALLOCATE RESOURCE").withWidth(40).withPadding(2);
}
