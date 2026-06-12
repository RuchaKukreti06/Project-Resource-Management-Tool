#include "admin/AdminScreen.h"
#include "admin/ManageEmployeesScreen.h"
#include "admin/ManageProjectsScreen.h"
#include "admin/ManageUsersScreen.h"
#include "admin/SystemConfigScreen.h"
#include "AuthSession.h"
#include <iomanip>
#include <iostream>
#include <unordered_map>

AdminScreen::AdminScreen()
{
}

void AdminScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    std::cout << "1. Manage Employees\n";
    std::cout << "2. Manage Projects\n";
    std::cout << "3. View All Allocations\n";
    std::cout << "4. Manage Users\n";
    std::cout << "5. System Configuration\n";
    std::cout << "6. Logout\n";
}

void AdminScreen::show(ApiClient& apiClient)
{
    while (true)
    {
        displayMenu();
        handleInput(apiClient);
        if (!api::AuthSession::instance().isLoggedIn())
        {
            break;
        }
    }
}

void AdminScreen::handleInput(ApiClient& apiClient)
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        ManageEmployeesScreen().show(apiClient);
    }
    else if (choice == "2")
    {
        ManageProjectsScreen().show(apiClient);
    }
    else if (choice == "3")
    {
        // View All Allocations
        try
        {
            // D5: Fetch all employees ONCE and build a lookup map (eliminates N+1)
            std::unordered_map<int, std::string> empNameMap;
            auto empRes = apiClient.get("/employees");
            if (empRes.contains("data"))
            {
                for (const auto& e : empRes["data"])
                    empNameMap[e["id"].get<int>()] = e["full_name"].get<std::string>();
            }

            auto response = apiClient.get("/projects");
            if (!response["success"].get<bool>())
            {
                showError(response["message"].get<std::string>());
                return;
            }
            auto projects = response["data"];
            // clearScreen();
            std::cout << "\n================ ALL ACTIVE ALLOCATIONS ================\n";
            std::cout << std::left << std::setw(15) << "Employee"
                      << std::setw(20) << "Project"
                      << std::setw(8)  << "%"
                      << std::setw(12) << "From"
                      << std::setw(12) << "To" << "\n";
            ScreenUtils::printDivider();

            int totalAllocations = 0;
            for (const auto& proj : projects)
            {
                int projId = proj["id"].get<int>();
                std::string projName = proj["name"].get<std::string>();

                // D4: Response is now {success, data}
                auto allocRes = apiClient.get("/projects/" + std::to_string(projId) + "/allocations");
                if (!allocRes["success"].get<bool>()) continue;

                for (const auto& alloc : allocRes["data"])
                {
                    int empId = alloc["employee_id"].get<int>();
                    // D5: O(1) map lookup instead of another GET /employees
                    std::string empName = "Emp " + std::to_string(empId);
                    auto it = empNameMap.find(empId);
                    if (it != empNameMap.end())
                        empName = it->second;

                    std::string toDate = alloc["to_date"].get<std::string>();
                    if (toDate.empty()) toDate = "Ongoing";

                    std::cout << std::left << std::setw(15) << empName.substr(0, 14)
                              << std::setw(20) << projName.substr(0, 19)
                              << std::setw(8)  << (std::to_string(alloc["utilization_percentage"].get<int>()) + "%")
                              << std::setw(12) << alloc["from_date"].get<std::string>()
                              << std::setw(12) << toDate << "\n";
                    totalAllocations++;
                }
            }
            ScreenUtils::printDivider();
            std::cout << "Total Active Allocations: " << totalAllocations << "\n\n";
            ScreenUtils::readLine("Press Enter to go back");
        }
        catch (const std::exception& e)
        {
            showError(std::string("Failed to fetch allocations: ") + e.what());
            ScreenUtils::readLine("Press Enter to continue");
        }
    }
    else if (choice == "4")
    {
        ManageUsersScreen().show(apiClient);
    }
    else if (choice == "5")
    {
        SystemConfigScreen().show(apiClient);
    }
    else if (choice == "6")
    {
        api::AuthSession::instance().logout();
        apiClient.clearToken();
        showSuccess("Logged out successfully.");
    }
    else
    {
        showError("Invalid option. Please enter 1–6.");
        ScreenUtils::readLine("Press Enter to continue");
    }
}

ScreenDecorator AdminScreen::decorator() const
{
    return ScreenDecorator("ADMIN PANEL").withWidth(40).withPadding(2);
}
