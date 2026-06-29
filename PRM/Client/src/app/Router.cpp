#include "Router.h"
#include "AppServices.h"
#include "screens/LoginScreen.h"
#include "screens/ChangePasswordScreen.h"
#include "screens/admin/AdminScreen.h"
#include "screens/admin/ManageUsersScreen.h"
#include "screens/admin/ManageEmployeesScreen.h"
#include "screens/admin/ManageProjectsScreen.h"
#include "screens/admin/AllAllocationsScreen.h"
#include "screens/admin/SystemConfigScreen.h"
#include "screens/manager/ManagerScreen.h"
#include "screens/manager/ResourceDashboardScreen.h"
#include "screens/manager/AllocateResourceScreen.h"
#include "screens/manager/MyProjectsScreen.h"
#include "screens/manager/TimesheetsScreen.h"
#include "screens/manager/AIAssistantScreen.h"
#include "screens/employee/EmployeeScreen.h"
#include "screens/employee/SubmitTimesheetScreen.h"
#include <iostream>

Router::Router(AppServices& services)
    : services_(services)
{
}

void Router::start()
{
    while (true)
    {
        if (!services_.sessionStore.isLoggedIn())
        {
            showLoginScreen();
        }
        else
        {
            routeAfterLogin();
        }
    }
}

void Router::routeAfterLogin()
{
    if (services_.sessionStore.forcePasswordChange())
    {
        showChangePasswordScreen();
        return; // Return to check loop (might logout if they fail, or they succeed and loop brings them back here)
    }

    const std::string& role = services_.sessionStore.role();
    if (role == "ADMIN")
    {
        routeToAdminMenu();
    }
    else if (role == "MANAGER")
    {
        routeToManagerMenu();
    }
    else if (role == "EMPLOYEE")
    {
        routeToEmployeeMenu();
    }
    else
    {
        std::cerr << "Unknown role: " << role << ". Logging out.\n";
        services_.sessionStore.logout();
        services_.apiClient.clearToken();
    }
}

void Router::showLoginScreen()
{
    LoginScreen screen(*this, *services_.authService, services_.sessionStore, services_.apiClient);
    screen.show();
}

void Router::showChangePasswordScreen()
{
    ChangePasswordScreen screen(*services_.authService, services_.sessionStore, services_.apiClient);
    screen.show();
}

void Router::routeToAdminMenu()
{
    AdminScreen screen(*this, services_.sessionStore);
    screen.show();
}

void Router::routeToManagerMenu()
{
    ManagerScreen screen(*this, services_.sessionStore);
    screen.show();
}

void Router::routeToEmployeeMenu()
{
    EmployeeScreen screen(*this, services_.sessionStore, *services_.employeeService, *services_.timesheetService, *services_.projectService, *services_.allocationService);
    screen.show();
}

void Router::navigateToManageUsers()
{
    ManageUsersScreen screen(*services_.userService, services_.sessionStore);
    screen.show();
}

void Router::navigateToManageEmployees()
{
    ManageEmployeesScreen screen(*services_.employeeService);
    screen.show();
}

void Router::navigateToManageProjects()
{
    ManageProjectsScreen screen(*services_.projectService);
    screen.show();
}

void Router::navigateToAllAllocations()
{
    AllAllocationsScreen screen(*services_.projectService, *services_.allocationService, *services_.employeeService);
    screen.show();
}

void Router::navigateToSystemConfig()
{
    SystemConfigScreen screen(*services_.configService);
    screen.show();
}

void Router::navigateToResourceDashboard()
{
    ResourceDashboardScreen screen(*services_.employeeService, *services_.projectService, *services_.allocationService, *services_.timesheetService, services_.sessionStore.userId());
    screen.show();
}

void Router::navigateToAllocateResource()
{
    AllocateResourceScreen screen(*services_.aiService, *services_.allocationService, *services_.projectService, *services_.employeeService, services_.sessionStore.userId());
    screen.show();
}

void Router::navigateToMyProjects()
{
    MyProjectsScreen screen(*services_.projectService, *services_.allocationService, *services_.employeeService, *services_.aiService, services_.sessionStore.userId());
    screen.show();
}

void Router::navigateToTimesheets()
{
    TimesheetsScreen screen(*services_.timesheetService, *services_.employeeService, services_.sessionStore.userId());
    screen.show();
}

void Router::navigateToAIAssistant()
{
    AIAssistantScreen screen(*services_.aiService, *services_.projectService, services_.sessionStore.userId());
    screen.show();
}

void Router::navigateToSubmitTimesheet()
{
    SubmitTimesheetScreen screen(*services_.timesheetService, *services_.allocationService, *services_.projectService, *services_.employeeService, services_.sessionStore);
    screen.show();
}


