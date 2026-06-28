#include "admin/AdminScreen.h"

#include "app/Router.h"
#include "utils/ConsoleInput.h"
#include <iostream>
#include <string>

using namespace AdminConstants;
using namespace AdminConstants::MainMenu;

AdminScreen::AdminScreen(Router& router, api::ISessionStore& sessionStore)
    : router_(router),
      sessionStore_(sessionStore)
{
}

void AdminScreen::displayMenu()
{
    decorator().render();
    std::cout << MANAGE_EMPLOYEES_OPTION << ". Manage Employees\n";
    std::cout << MANAGE_PROJECTS_OPTION << ". Manage Projects\n";
    std::cout << VIEW_ALLOCATIONS_OPTION << ". View All Allocations\n";
    std::cout << MANAGE_USERS_OPTION << ". Manage Users\n";
    std::cout << SYSTEM_CONFIG_OPTION << ". System Configuration\n";
    std::cout << LOGOUT_OPTION << ". Logout\n";
}

void AdminScreen::show()
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

void AdminScreen::handleInput()
{
    std::string choice = ConsoleInput::readLine("Enter option");
    if (choice == MANAGE_EMPLOYEES_OPTION)
    {
        router_.navigateToManageEmployees();
    }
    else if (choice == MANAGE_PROJECTS_OPTION)
    {
        router_.navigateToManageProjects();
    }
    else if (choice == VIEW_ALLOCATIONS_OPTION)
    {
        router_.navigateToAllAllocations();
    }
    else if (choice == MANAGE_USERS_OPTION)
    {
        router_.navigateToManageUsers();
    }
    else if (choice == SYSTEM_CONFIG_OPTION)
    {
        router_.navigateToSystemConfig();
    }
    else if (choice == LOGOUT_OPTION)
    {
        sessionStore_.logout();
        showSuccess("Logged out successfully.");
    }
    else
    {
        showError("Invalid option. Please enter 1–6.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

ScreenDecorator AdminScreen::decorator() const
{
    return ScreenDecorator("ADMIN PANEL").withWidth(DEFAULT_PANEL_WIDTH).withPadding(DEFAULT_PADDING);
}
