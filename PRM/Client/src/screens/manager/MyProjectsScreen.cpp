#include "manager/MyProjectsScreen.h"
#include "AuthSession.h"
#include <iomanip>
#include <ctime>

MyProjectsScreen::MyProjectsScreen()
{
}

void MyProjectsScreen::displayMenu()
{
}

void MyProjectsScreen::show(ApiClient& apiClient)
{
    while (true)
    {
        try
        {
            int managerId = api::AuthSession::instance().userId();
            auto response = apiClient.get("/managers/" + std::to_string(managerId) + "/projects");
            
            if (!response["success"].get<bool>())
            {
                showError(response["message"].get<std::string>());
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }

            auto projects = response["data"];
            // clearScreen();
            decorator().render();

            std::cout << std::left << std::setw(6) << "#"
                      << std::setw(25) << "Project"
                      << std::setw(15) << "End Date"
                      << "Health\n";
            ScreenUtils::printDivider();

            std::vector<nlohmann::json> projList;
            int idx = 1;
            for (const auto& proj : projects)
            {
                projList.push_back(proj);
                
                // Fetch milestones to compute health
                int projId = proj["id"].get<int>();
                auto msRes = apiClient.get("/projects/" + std::to_string(projId) + "/milestones");
                
                std::string health = "🟢 ON TRACK";
                bool hasOverdue = false;
                bool hasApproaching = false;
                
                std::time_t now = std::time(nullptr);
                std::tm local = {};
#ifdef _WIN32
                localtime_s(&local, &now);
#else
                local = *std::localtime(&now);
#endif
                char todayBuffer[11] = {0};
                std::strftime(todayBuffer, sizeof(todayBuffer), "%Y-%m-%d", &local);
                std::string todayStr(todayBuffer);

                std::time_t nextWeek = now + (7 * 24 * 60 * 60);
                std::tm localNext = {};
#ifdef _WIN32
                localtime_s(&localNext, &nextWeek);
#else
                localNext = *std::localtime(&nextWeek);
#endif
                char nextBuffer[11] = {0};
                std::strftime(nextBuffer, sizeof(nextBuffer), "%Y-%m-%d", &localNext);
                std::string nextStr(nextBuffer);

                if (msRes["success"].get<bool>())
                {
                    for (const auto& ms : msRes["data"])
                    {
                        std::string status = ms["status"].get<std::string>();
                        std::string dueDate = ms["due_date"].get<std::string>();
                        if (status != "DONE" && dueDate < todayStr)
                        {
                            hasOverdue = true;
                        }
                        else if (status != "DONE" && dueDate <= nextStr)
                        {
                            hasApproaching = true;
                        }
                    }
                }

                if (hasOverdue)
                {
                    health = "🔴 AT RISK";
                }
                else if (hasApproaching)
                {
                    health = "🟡 ATTENTION";
                }

                std::cout << std::left << std::setw(6) << idx++
                          << std::setw(25) << proj["name"].get<std::string>().substr(0, 24)
                          << std::setw(15) << proj["end_date"].get<std::string>()
                          << health << "\n";
            }
            ScreenUtils::printDivider();

            if (projList.empty())
            {
                showInfo("You have no assigned projects.");
                ScreenUtils::readLine("Press Enter to go back");
                break;
            }

            std::string selStr = ScreenUtils::readLine("Select project number to view details (or 0 to go back)");
            int selection = std::stoi(selStr);
            if (selection == 0)
            {
                break;
            }
            if (selection >= 1 && selection <= (int)projList.size())
            {
                viewProjectDetail(apiClient, projList[selection - 1]["id"].get<int>());
            }
        }
        catch (const std::exception& ex)
        {
            showError(std::string("Error viewing projects: ") + ex.what());
            ScreenUtils::readLine("Press Enter to continue");
            break;
        }
    }
}

void MyProjectsScreen::handleInput(ApiClient& apiClient)
{
}

void MyProjectsScreen::viewProjectDetail(ApiClient& apiClient, int projectId)
{
    while (true)
    {
        try
        {
            auto projRes = apiClient.get("/projects/" + std::to_string(projectId));
            if (!projRes["success"].get<bool>()) return;
            auto proj = projRes["data"];

            auto msRes = apiClient.get("/projects/" + std::to_string(projectId) + "/milestones");
            auto allocsRes = apiClient.get("/projects/" + std::to_string(projectId) + "/allocations");

            // clearScreen();
            std::cout << "\n── " << proj["name"].get<std::string>() << " ───────────────────────────────\n";
            
            // Health flags
            std::time_t now = std::time(nullptr);
            std::tm local = {};
#ifdef _WIN32
            localtime_s(&local, &now);
#else
            local = *std::localtime(&now);
#endif
            char todayBuffer[11] = {0};
            std::strftime(todayBuffer, sizeof(todayBuffer), "%Y-%m-%d", &local);
            std::string todayStr(todayBuffer);

            std::time_t nextWeek = now + (7 * 24 * 60 * 60);
            std::tm localNext = {};
#ifdef _WIN32
            localtime_s(&localNext, &nextWeek);
#else
            localNext = *std::localtime(&nextWeek);
#endif
            char nextBuffer[11] = {0};
            std::strftime(nextBuffer, sizeof(nextBuffer), "%Y-%m-%d", &localNext);
            std::string nextStr(nextBuffer);

            bool hasOverdue = false;
            bool hasApproaching = false;
            std::vector<std::string> overdueTitles;
            if (msRes["success"].get<bool>())
            {
                for (const auto& ms : msRes["data"])
                {
                    if (ms["status"].get<std::string>() != "DONE" && ms["due_date"].get<std::string>() < todayStr)
                    {
                        hasOverdue = true;
                        overdueTitles.push_back(ms["title"].get<std::string>());
                    }
                    else if (ms["status"].get<std::string>() != "DONE" && ms["due_date"].get<std::string>() <= nextStr)
                    {
                        hasApproaching = true;
                    }
                }
            }

            std::string healthStatus = "🟢 ON TRACK";
            if (hasOverdue) healthStatus = "🔴 AT RISK";
            else if (hasApproaching) healthStatus = "🟡 ATTENTION";
            
            std::cout << "Health Status : " << healthStatus << "\n\n";
            if (hasOverdue)
            {
                std::cout << "Risk Flags:\n";
                for (const auto& title : overdueTitles)
                {
                    std::cout << "  ✗ " << title << " milestone is overdue\n";
                }
                std::cout << "\n";
            }

            std::cout << "Milestones:\n";
            std::cout << std::left << std::setw(6) << "  #"
                      << std::setw(25) << "Title"
                      << std::setw(15) << "Due Date"
                      << "Status\n";
            ScreenUtils::printDivider();

            if (msRes["success"].get<bool>())
            {
                int mIdx = 1;
                for (const auto& ms : msRes["data"])
                {
                    std::cout << "  " << std::left << std::setw(4) << mIdx++
                              << std::setw(25) << ms["title"].get<std::string>().substr(0, 24)
                              << std::setw(15) << ms["due_date"].get<std::string>()
                              << ms["status"].get<std::string>() << "\n";
                }
            }
            std::cout << "\nAllocated Resources:\n";
            std::cout << std::left << std::setw(20) << "  Name"
                      << std::setw(8) << "%"
                      << std::setw(12) << "From"
                      << std::setw(12) << "To" << "\n";
            ScreenUtils::printDivider();

            for (const auto& alloc : (allocsRes.contains("data") && allocsRes["data"].is_array() ? allocsRes["data"] : nlohmann::json::array()))
            {
                int empId = alloc["employee_id"].get<int>();
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

                std::cout << "  " << std::left << std::setw(18) << empName.substr(0, 17)
                          << std::setw(8) << (std::to_string(alloc["utilization_percentage"].get<int>()) + "%")
                          << std::setw(12) << alloc["from_date"].get<std::string>()
                          << std::setw(12) << alloc["to_date"].get<std::string>() << "\n";
            }
            std::cout << "\n";

            std::cout << "[A] Get AI Risk Summary     [B] Back\n";
            std::string choice = ScreenUtils::readLine("Enter option");
            if (choice == "A" || choice == "a")
            {
                std::cout << "\nGenerating AI summary...\n\n";
                nlohmann::json payload = {{"project_id", projectId}};
                auto aiResponse = apiClient.post("/ai/risk-summary", payload);

                std::cout << "\n── AI Risk Summary — " << proj["name"].get<std::string>() << " ────────────\n\n";

                if (aiResponse.contains("success") && aiResponse["success"].get<bool>())
                {
                    std::cout << "\"" << aiResponse["data"]["summary"].get<std::string>() << "\"\n";
                }
                else
                {
                    std::cout << "AI service error. Please check the API key in System Configuration.\n";
                }

                std::cout << "\n  Note: This summary is AI-generated from milestone and timesheet data.\n\n";
                ScreenUtils::readLine("Press Enter to go back");
            }
            else if (choice == "B" || choice == "b")
            {
                break;
            }
        }
        catch (const std::exception& ex)
        {
            showError(std::string("Error viewing project detail: ") + ex.what());
            ScreenUtils::readLine("Press Enter to continue");
            break;
        }
    }
}

ScreenDecorator MyProjectsScreen::decorator() const
{
    return ScreenDecorator("MY PROJECTS").withWidth(40).withPadding(2);
}
