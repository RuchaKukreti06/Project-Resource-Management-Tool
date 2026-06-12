#include "employee/SubmitTimesheetScreen.h"
#include "AuthSession.h"
#include <iomanip>

SubmitTimesheetScreen::SubmitTimesheetScreen()
{
}

void SubmitTimesheetScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
}

void SubmitTimesheetScreen::show(ApiClient& apiClient)
{
    displayMenu();
    handleInput(apiClient);
}

void SubmitTimesheetScreen::handleInput(ApiClient& apiClient)
{
    try
    {
        // Find Employee ID
        int userId = api::AuthSession::instance().userId();
        int empId = 0;
        auto empRes = apiClient.get("/employees");
        if (empRes["success"].get<bool>())
        {
            for (const auto& e : empRes["data"])
            {
                if (e["user_id"].get<int>() == userId)
                {
                    empId = e["id"].get<int>();
                    break;
                }
            }
        }

        if (empId == 0)
        {
            showError("No employee profile linked to your user account.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        std::cout << "Week Start: Enter date (DD-MM-YYYY) or press Enter for last Monday\n";
        std::string weekInput = ScreenUtils::readLine("Week");
        
        std::string weekStart = weekInput;
        if (weekInput.empty())
        {
            std::time_t now = std::time(nullptr);
            std::tm local = {};
#ifdef _WIN32
            localtime_s(&local, &now);
#else
            local = *std::localtime(&now);
#endif
            int daysSinceMonday = local.tm_wday == 0 ? 6 : local.tm_wday - 1;
            std::time_t lastMondayTime = now - ((daysSinceMonday + 7) * 24 * 60 * 60);
            std::tm lastMonday = {};
#ifdef _WIN32
            localtime_s(&lastMonday, &lastMondayTime);
#else
            lastMonday = *std::localtime(&lastMondayTime);
#endif
            char buffer[11] = {0};
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &lastMonday);
            weekStart = buffer;
        }
        else if (weekInput.length() == 10 && weekInput[2] == '-' && weekInput[5] == '-')
        {
            weekStart = weekInput.substr(6, 4) + "-" + weekInput.substr(3, 2) + "-" + weekInput.substr(0, 2);
        }        std::cout << "\nChecking your active allocations for this week...\n";

        // Query active projects/allocations
        auto projRes = apiClient.get("/projects");
        std::vector<nlohmann::json> activeAllocs;

        if (projRes["success"].get<bool>())
        {
            for (const auto& proj : projRes["data"])
            {
                int projId = proj["id"].get<int>();
                auto allocs = apiClient.get("/projects/" + std::to_string(projId) + "/allocations");
                for (const auto& alloc : allocs)
                {
                    if (alloc["employee_id"].get<int>() == empId)
                    {
                        std::string fromD = alloc["from_date"].get<std::string>();
                        std::string toD = alloc.value("to_date", "");
                        
                        // To be active, it must start on or before the week end date, and end on or after the week start date.
                        // weekStart + 6 days = weekEnd (approx).
                        // simpler logic: check if from_date <= weekStart and (toD is empty or toD >= weekStart)
                        // A truly accurate check is backend's getActiveAllocationsForWeek, but this local approximation works for client display.
                        if (fromD <= weekStart && (toD.empty() || toD >= weekStart))
                        {
                            activeAllocs.push_back({
                                {"project_id", projId},
                                {"project_name", proj["name"].get<std::string>()},
                                {"utilisation_percent", alloc["utilization_percentage"].get<int>()}
                            });
                        }
                    }
                }
            }
        }

        if (activeAllocs.empty())
        {
            showInfo("No active allocations found. Standard 40 hours will be logged to Bench/Operations.");
            // Log to Bench Project
            // Find a project named Bench or Operations
            int benchProjId = 0;
            if (projRes["success"].get<bool>())
            {
                for (const auto& p : projRes["data"])
                {
                    std::string pName = p["name"].get<std::string>();
                    if (pName == "Bench" || pName == "Operations")
                    {
                        benchProjId = p["id"].get<int>();
                        break;
                    }
                }
            }
            if (benchProjId == 0)
            {
                showError("No Bench or Operations project found to log hours.");
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }
            activeAllocs.push_back({
                {"project_id", benchProjId},
                {"project_name", "Bench/Operations"},
                {"utilisation_percent", 100}
            });
        }

        nlohmann::json timesheetLines = nlohmann::json::array();
        int totalHours = 0;

        for (int i = 0; i < (int)activeAllocs.size(); ++i)
        {
            auto alloc = activeAllocs[i];
            int expected = (alloc["utilisation_percent"].get<int>() * 40) / 100;
            std::cout << "\n──────────────────────────────────────────────\n";
            std::cout << "PROJECT " << (i + 1) << " OF " << activeAllocs.size() << " — " << alloc["project_name"].get<std::string>() << "\n";
            std::cout << "  Allocation: " << alloc["utilisation_percent"].get<int>() << "%   |   Expected: " << expected << " hrs max\n";
            std::cout << "──────────────────────────────────────────────\n";

            std::string hrsStr = ScreenUtils::readLine("Hours worked this week");
            int hours = std::stoi(hrsStr);
            totalHours += hours;

            std::cout << "\nWhat did you work on? Select activity tags:\n";
            std::cout << "  1. Backend API Development\n";
            std::cout << "  2. Microservices / Architecture\n";
            std::cout << "  3. Database Design & Queries\n";
            std::cout << "  4. WebSocket / Real-time Features\n";
            std::cout << "  5. Frontend Development\n";
            std::cout << "  6. Code Review / Mentoring\n";
            std::cout << "  7. Bug Fixing\n";
            std::cout << "  8. DevOps / Deployment\n";
            std::cout << "  9. Testing & QA\n";
            std::cout << "  10. Documentation\n";
            std::cout << "  11. Other\n";

            std::string tagsInput = ScreenUtils::readLine("Select tags (comma-separated, e.g., 1,7)");
            
            // Map tag choices to string values
            std::vector<std::string> tags;
            std::stringstream ss(tagsInput);
            std::string item;
            while (std::getline(ss, item, ','))
            {
                if (item == "1") tags.push_back("Backend API Development");
                else if (item == "2") tags.push_back("Microservices / Architecture");
                else if (item == "3") tags.push_back("Database Design & Queries");
                else if (item == "4") tags.push_back("WebSocket / Real-time Features");
                else if (item == "5") tags.push_back("Frontend Development");
                else if (item == "6") tags.push_back("Code Review / Mentoring");
                else if (item == "7") tags.push_back("Bug Fixing");
                else if (item == "8") tags.push_back("DevOps / Deployment");
                else if (item == "9") tags.push_back("Testing & QA");
                else if (item == "10") tags.push_back("Documentation");
                else tags.push_back("Other Development");
            }

            timesheetLines.push_back({
                {"project_id", alloc["project_id"].get<int>()},
                {"hours_worked", hours},
                {"tags", tags}
            });
        }

        // clearScreen();
        std::cout << "\n================ TIMESHEET SUMMARY ================\n";
        for (const auto& line : timesheetLines)
        {
            int pId = line["project_id"].get<int>();
            std::string pName = "Proj " + std::to_string(pId);
            for (const auto& a : activeAllocs)
            {
                if (a["project_id"].get<int>() == pId)
                {
                    pName = a["project_name"].get<std::string>();
                    break;
                }
            }

            std::cout << "  " << std::left << std::setw(25) << pName.substr(0, 24)
                      << line["hours_worked"].get<int>() << " hrs  [";
            int tCount = 0;
            for (const auto& t : line["tags"])
            {
                if (tCount > 0) std::cout << ", ";
                std::cout << t.get<std::string>();
                tCount++;
            }
            std::cout << "]\n";
        }
        std::cout << "  ────────────────────────────────────────────\n";
        std::cout << "  Total           " << totalHours << " hrs / 40 hrs max   " << (totalHours <= 40 ? "✓" : "⚠") << "\n\n";

        std::cout << "[S] Submit Timesheet      [B] Back\n";
        std::string submitChoice = ScreenUtils::readLine("Choice");
        if (submitChoice == "S" || submitChoice == "s")
        {
            auto response = apiClient.post("/timesheets", {
                {"employee_id", empId},
                {"week_start_date", weekStart},
                {"lines", timesheetLines},
                {"max_weekly_hours", 40}
            });

            if (response["success"].get<bool>())
            {
                showSuccess("Timesheet submitted successfully! ✓");
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
        showError(std::string("Error during timesheet submission: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

ScreenDecorator SubmitTimesheetScreen::decorator() const
{
    return ScreenDecorator("SUBMIT TIMESHEET").withWidth(40).withPadding(2);
}
