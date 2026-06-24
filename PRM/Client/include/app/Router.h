#ifndef ROUTER_H
#define ROUTER_H

#include <string>

struct AppServices;

class Router
{
private:
    AppServices& services_;

public:
    explicit Router(AppServices& services);

    void start();

    // Core navigation
    void routeAfterLogin();
    void showLoginScreen();
    void showChangePasswordScreen();
    
    // Role menus
    void routeToAdminMenu();
    void routeToManagerMenu();
    void routeToEmployeeMenu();

    // Admin routes
    void navigateToManageUsers();
    void navigateToManageEmployees();
    void navigateToManageProjects();
    void navigateToSystemConfig();

    // Manager routes
    void navigateToResourceDashboard();
    void navigateToAllocateResource();
    void navigateToMyProjects();
    void navigateToTimesheets();
    void navigateToAIAssistant();

    // Employee routes
    void navigateToSubmitTimesheet();
};

#endif // ROUTER_H
