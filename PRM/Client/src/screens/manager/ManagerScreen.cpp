#include "manager/ManagerScreen.h"
#include "app/Router.h"
#include <iostream>
#include <string>
#include "screens/ScreenUtils.h"

using namespace ManagerConstants;
using namespace ManagerConstants::MainMenu;

ManagerScreen::ManagerScreen(Router& router, api::ISessionStore& sessionStore)
    : router_(router), sessionStore_(sessionStore)
{
}

void ManagerScreen::displayMenu()
{
    decorator().render();
    std::cout << OPT_RESOURCE_DASHBOARD << ". Resource Dashboard\n";
    std::cout << OPT_ALLOCATE_RESOURCE << ". Allocate Resource\n";
    std::cout << OPT_MY_PROJECTS << ". My Projects\n";
    std::cout << OPT_TIMESHEETS << ". Timesheets\n";
    std::cout << OPT_AI_ASSISTANT << ". AI Assistant\n";
    std::cout << OPT_LOGOUT << ". Logout\n";
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
    if (choice == OPT_RESOURCE_DASHBOARD)
    {
        router_.navigateToResourceDashboard();
    }
    else if (choice == OPT_ALLOCATE_RESOURCE)
    {
        router_.navigateToAllocateResource();
    }
    else if (choice == OPT_MY_PROJECTS)
    {
        router_.navigateToMyProjects();
    }
    else if (choice == OPT_TIMESHEETS)
    {
        router_.navigateToTimesheets();
    }
    else if (choice == OPT_AI_ASSISTANT)
    {
        router_.navigateToAIAssistant();
    }
    else if (choice == OPT_LOGOUT)
    {
        sessionStore_.logout();
        showSuccess("Logged out successfully.");
    }
    else
    {
        showError(Messages::INVALID_OPTION);
        ScreenUtils::readLine(Messages::PRESS_ENTER_TO_CONTINUE);
    }
}

ScreenDecorator ManagerScreen::decorator() const
{
    std::string username = sessionStore_.username();
    return ScreenDecorator("Welcome, " + username + "!")
        .withWidth(ManagerConstants::DEFAULT_PANEL_WIDTH)
        .withPadding(ManagerConstants::DEFAULT_PADDING);
}
