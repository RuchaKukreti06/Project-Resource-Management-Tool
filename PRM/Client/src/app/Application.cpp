#include "Application.h"

#include <iostream>

#include "ApiClient.h"
#include "AuthSession.h"
#include "LoginScreen.h"
#include "admin/AdminScreen.h"
#include "employee/EmployeeScreen.h"
#include "manager/ManagerScreen.h"

Application::Application(const std::string& baseUrl, ApiClient& apiClient)
    : apiClient(apiClient), baseUrl(baseUrl)
{
}

void Application::run()
{
    while (true)
    {
        if (!api::AuthSession::instance().isLoggedIn())
        {
            LoginScreen loginScreen;
            loginScreen.show(apiClient);

            // std::cout << "Welcome, " << api::AuthSession::instance().username() << "!\n";

            std::unique_ptr<Screen> screen;

            const auto& role = api::AuthSession::instance().role();
            if (role == "ADMIN")
            {
                screen = std::make_unique<AdminScreen>();
            }
            else if (role == "MANAGER")
            {
                screen = std::make_unique<ManagerScreen>();
            }
            else
            {
                screen = std::make_unique<EmployeeScreen>();
            }
            screen->show(apiClient);
        }
    }
}
