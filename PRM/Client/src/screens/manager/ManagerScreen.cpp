#include "manager/ManagerScreen.h"
#include "manager/ResourceDashboardScreen.h"
#include "manager/AllocateResourceScreen.h"
#include "manager/MyProjectsScreen.h"
#include "manager/TimesheetsScreen.h"
#include "manager/AIAssistantScreen.h"
#include "AuthSession.h"

ManagerScreen::ManagerScreen()
{
}

void ManagerScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    std::cout << "1. Resource Dashboard\n";
    std::cout << "2. Allocate Resource\n";
    std::cout << "3. My Projects\n";
    std::cout << "4. Timesheets\n";
    std::cout << "5. AI Assistant\n";
    std::cout << "6. Logout\n";
}

void ManagerScreen::show(ApiClient& apiClient)
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

void ManagerScreen::handleInput(ApiClient& apiClient)
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        ResourceDashboardScreen().show(apiClient);
    }
    else if (choice == "2")
    {
        AllocateResourceScreen().show(apiClient);
    }
    else if (choice == "3")
    {
        MyProjectsScreen().show(apiClient);
    }
    else if (choice == "4")
    {
        TimesheetsScreen().show(apiClient);
    }
    else if (choice == "5")
    {
        AIAssistantScreen().show(apiClient);
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

ScreenDecorator ManagerScreen::decorator() const
{
    std::string username = api::AuthSession::instance().username();
    return ScreenDecorator("Welcome, " + username + "!").withWidth(40).withPadding(2);
}
