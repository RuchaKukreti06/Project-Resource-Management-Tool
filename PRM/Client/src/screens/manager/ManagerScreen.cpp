#include "manager/ManagerScreen.h"
#include "manager/ResourceDashboardScreen.h"
#include "manager/AllocateResourceScreen.h"
#include "manager/MyProjectsScreen.h"
#include "manager/TimesheetsScreen.h"
#include "manager/AIAssistantScreen.h"
#include "app/Router.h"

ManagerScreen::ManagerScreen(Router& router, api::ISessionStore& sessionStore)
    : router_(router), sessionStore_(sessionStore)
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

void ManagerScreen::show()
{
    while (true)
    {
        displayMenu();
        handleInput();
        if (!sessionStore_.isLoggedIn())
        {
            break;
        }
    }
}

void ManagerScreen::handleInput()
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        router_.navigateToResourceDashboard();
    }
    else if (choice == "2")
    {
        router_.navigateToAllocateResource();
    }
    else if (choice == "3")
    {
        router_.navigateToMyProjects();
    }
    else if (choice == "4")
    {
        router_.navigateToTimesheets();
    }
    else if (choice == "5")
    {
        router_.navigateToAIAssistant();
    }
    else if (choice == "6")
    {
        sessionStore_.logout();
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
    std::string username = sessionStore_.username();
    return ScreenDecorator("Welcome, " + username + "!").withWidth(40).withPadding(2);
}
