#include "LoginScreen.h"

#include "Router.h"
#include "dto/AuthDTO.h"
#include "api/ApiException.h"
#include "services/AuthClientService.h"

LoginScreen::LoginScreen(Router& router, AuthClientService& authService, api::ISessionStore& sessionStore, IApiClient& apiClient)
    : router_(router), authService_(authService), sessionStore_(sessionStore), apiClient_(apiClient)
{
}

ScreenDecorator LoginScreen::decorator() const
{
    return ScreenDecorator("PROJECT & RESOURCE MANAGEMENT TOOL", "Learn & Code — Final Project")
        .withWidth(50)
        .withPadding(2);
}

void LoginScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    ScreenOptions({"Login", "Exit"}).render();
}

void LoginScreen::show()
{
    displayMenu();
    handleInput();
}

void LoginScreen::handleInput()
{
    while (true)
    {
        std::string choice = ScreenUtils::readLine("Enter option");
        if (choice == "1")
        {
            std::string username = ScreenUtils::readLine("Username");
            std::string password = ScreenUtils::readPassword("Password");
            if (username.empty())
            {
                showError("Username cannot be empty. Please try again.");
                ScreenUtils::readLine("Press Enter to continue");
                displayMenu();
                continue;
            }
            if (password.empty())
            {
                showError("Password cannot be empty. Please try again.");
                ScreenUtils::readLine("Press Enter to continue");
                displayMenu();
                continue;
            }

            try
            {
                auto loginRes = authService_.login(username, password);
                if (!loginRes.success)
                {
                    showError(loginRes.message);
                    ScreenUtils::readLine("Press Enter to continue");
                    displayMenu();
                    continue;
                }

                std::string token = loginRes.token;
                std::string respUsername = loginRes.user.username;
                std::string role = loginRes.user.role;
                int userId = loginRes.user.id;
                bool forcePasswordChange = loginRes.user.forcePasswordChange;

                apiClient_.setToken(token);
                sessionStore_.login(token, respUsername, role, userId, forcePasswordChange);

                if (forcePasswordChange)
                {
                    showInfo("Password change is required on first login.");
                    // Router will handle navigation to ChangePasswordScreen when LoginScreen returns
                }
                else
                {
                    showSuccess("Login successful.");
                }
                break;
            }
            catch (const ApiException& ex)
            {
                showError(ex.what());
                ScreenUtils::readLine("Press Enter to continue");
                displayMenu();
            }
            catch (const std::exception&)
            {
                showError("Something went wrong. Please try again.");
            }
        }
        else if (choice == "2")
        {
            std::cout << "Goodbye.\n";
            std::exit(0);
        }
        else
        {
            showError("Please select a valid option.");
            ScreenUtils::readLine("Press Enter to continue");
            displayMenu();
        }
    }
}
