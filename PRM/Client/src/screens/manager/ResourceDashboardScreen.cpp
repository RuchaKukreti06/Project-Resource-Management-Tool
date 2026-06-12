#include "manager/ResourceDashboardScreen.h"
#include "AuthSession.h"
#include <iomanip>
#include <set>
#include <sstream>

ResourceDashboardScreen::ResourceDashboardScreen()
{
}

void ResourceDashboardScreen::displayMenu()
{
    // Display is handled in show() dynamically
}

void ResourceDashboardScreen::show(ApiClient& apiClient)
{
    while (true)
    {
        try
        {
            int managerId = api::AuthSession::instance().userId();
            auto response = apiClient.get("/managers/" + std::to_string(managerId) + "/team");
            if (!response["success"].get<bool>())
            {
                showError(response["message"].get<std::string>());
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }

            auto employees = response["data"];
            // clearScreen();
            decorator().render();

            std::cout << "\nON BENCH\n";
            std::cout << std::left << std::setw(6) << "ID"
                      << std::setw(20) << "Name"
                      << std::setw(15) << "Department"
                      << "Skills\n";
            ScreenUtils::printDivider();

            int benchCount = 0;
            int activeCount = 0;

            std::vector<nlohmann::json> activeList;

            for (const auto& emp : employees)
            {
                if (!emp["is_active"].get<bool>()) continue;

                int empId = emp["id"].get<int>();
                std::string status = emp["status"].get<std::string>();

                if (status == "BENCH")
                {
                    // Fetch skills
                    std::string skillsStr = "";
                    auto skillsRes = apiClient.get("/employees/" + std::to_string(empId) + "/skills");
                    if (skillsRes["success"].get<bool>())
                    {
                        int count = 0;
                        for (const auto& s : skillsRes["data"])
                        {
                            if (count > 0) skillsStr += ", ";
                            skillsStr += s["skill_name"].get<std::string>();
                            count++;
                        }
                    }

                    std::cout << std::left << std::setw(6) << empId
                              << std::setw(20) << emp["full_name"].get<std::string>().substr(0, 19)
                              << std::setw(15) << emp["department"].get<std::string>().substr(0, 14)
                              << skillsStr << "\n";
                    benchCount++;
                }
                else
                {
                    activeList.push_back(emp);
                    activeCount++;
                }
            }

            std::cout << "\nACTIVE EMPLOYEES\n";
            std::cout << std::left << std::setw(6) << "ID"
                      << std::setw(20) << "Name"
                      << std::setw(15) << "Alloc %"
                      << "Availability\n";
            ScreenUtils::printDivider();

            for (const auto& emp : activeList)
            {
                int util = emp.value("total_utilisation", 0);
                std::string avail = (util == 0) ? "FULL" : (util >= 100 ? "0% free" : std::to_string(100 - util) + "% free");

                std::cout << std::left << std::setw(6) << emp["id"].get<int>()
                          << std::setw(20) << emp["full_name"].get<std::string>().substr(0, 19)
                          << std::setw(15) << (std::to_string(util) + "%")
                          << avail << "\n";
            }
            ScreenUtils::printDivider();
            std::cout << "Bench: " << benchCount << "   |   Active: " << activeCount << "\n\n";

            std::cout << "[D] Drill into employee details     [B] Back\n";
            std::string choice = ScreenUtils::readLine("Enter choice");
            if (choice == "D" || choice == "d")
            {
                drillIntoEmployeeDetails(apiClient);
            }
            else if (choice == "B" || choice == "b")
            {
                break;
            }
        }
        catch (const std::exception& ex)
        {
            showError(std::string("Dashboard error: ") + ex.what());
            ScreenUtils::readLine("Press Enter to continue");
            break;
        }
    }
}

void ResourceDashboardScreen::handleInput(ApiClient& apiClient)
{
}

void ResourceDashboardScreen::drillIntoEmployeeDetails(ApiClient& apiClient)
{
    try
    {
        std::string empId = ScreenUtils::readLine("Enter Employee ID");
        int managerId = api::AuthSession::instance().userId();
        auto response = apiClient.get("/managers/" + std::to_string(managerId) + "/team");
        if (!response["success"].get<bool>()) return;

        nlohmann::json targetEmp;
        bool found = false;
        for (const auto& emp : response["data"])
        {
            if (std::to_string(emp["id"].get<int>()) == empId)
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
        std::cout << "\n── " << targetEmp["full_name"].get<std::string>() << " ─────────────────────────────────\n";
        std::cout << "Department     : " << targetEmp["department"].get<std::string>() << "\n";
        std::cout << "Designation    : " << targetEmp["designation"].get<std::string>() << "\n";
        std::cout << "Current Status : " << targetEmp["status"].get<std::string>() << "\n";

        // Fetch skills
        std::string skillsStr = "";
        auto skillsRes = apiClient.get("/employees/" + empId + "/skills");
        if (skillsRes["success"].get<bool>())
        {
            int count = 0;
            for (const auto& s : skillsRes["data"])
            {
                if (count > 0) skillsStr += ", ";
                skillsStr += s["skill_name"].get<std::string>() + " (" + s["proficiency"].get<std::string>() + ")";
                count++;
            }
        }
        std::cout << "Profile Skills : " << (skillsStr.empty() ? "None" : skillsStr) << "\n\n";

        // Fetch active allocations across all projects
        std::cout << "Active Allocations:\n";
        std::cout << std::left << std::setw(20) << "  Project" 
                  << std::setw(8) << "%" 
                  << std::setw(12) << "From" 
                  << std::setw(12) << "To" << "\n";
        ScreenUtils::printDivider();

        auto projResponse = apiClient.get("/projects");
        if (projResponse["success"].get<bool>())
        {
            for (const auto& proj : projResponse["data"])
            {
                int projId = proj["id"].get<int>();
                auto allocsRes = apiClient.get("/projects/" + std::to_string(projId) + "/allocations");
                if (!allocsRes.contains("data") || !allocsRes["data"].is_array()) continue;
                for (const auto& alloc : allocsRes["data"])
                {
                    if (std::to_string(alloc["employee_id"].get<int>()) == empId)
                    {
                        std::cout << "  " << std::left << std::setw(18) << proj["name"].get<std::string>().substr(0, 17)
                                  << std::setw(8) << (std::to_string(alloc["utilization_percentage"].get<int>()) + "%")
                                  << std::setw(12) << alloc["from_date"].get<std::string>()
                                  << std::setw(12) << alloc["to_date"].get<std::string>() << "\n";
                    }
                }
            }
        }
        std::cout << "\n";

        // Fetch recent activity tags from timesheets (real data)
        std::cout << "Recent Activity Tags (last 4 weeks):\n  ";
        auto tsRes = apiClient.get("/employees/" + empId + "/timesheets");
        std::set<std::string> tagSet;
        if (tsRes["success"].get<bool>() && tsRes["data"].is_array())
        {
            int weekCount = 0;
            for (const auto& ts : tsRes["data"])
            {
                if (weekCount++ >= 4) break;
                auto detailRes = apiClient.get("/timesheets/" + std::to_string(ts["id"].get<int>()));
                if (detailRes["success"].get<bool>() && detailRes["data"].is_array())
                {
                    for (const auto& row : detailRes["data"])
                    {
                        std::string tags = row.value("tags", "");
                        std::istringstream ss(tags);
                        std::string tag;
                        while (std::getline(ss, tag, ','))
                        {
                            auto start = tag.find_first_not_of(" ");
                            if (start != std::string::npos)
                                tagSet.insert(tag.substr(start));
                        }
                    }
                }
            }
        }
        std::string tagsStr;
        for (const auto& t : tagSet) { if (!tagsStr.empty()) tagsStr += ", "; tagsStr += t; }
        std::cout << (tagsStr.empty() ? "No recent activity recorded." : tagsStr) << "\n\n";

        ScreenUtils::readLine("Press Enter to go back");
    }
    catch (const std::exception& ex)
    {
        showError(std::string("Error drilling into employee details: ") + ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
}

ScreenDecorator ResourceDashboardScreen::decorator() const
{
    return ScreenDecorator("RESOURCE DASHBOARD").withWidth(40).withPadding(2);
}
