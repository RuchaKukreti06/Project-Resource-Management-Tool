#include "LoginScreen.h"

#include "AuthSession.h"
#include "ChangePasswordScreen.h"
#include "dto/AuthDTO.h"

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

void LoginScreen::show(ApiClient& apiClient)
{
    displayMenu();
    handleInput(apiClient);
}

void LoginScreen::handleInput(ApiClient& apiClient)
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
                auto response = apiClient.post(
                    "/auth/login", {{"username", username}, {"password", password}});
                auto loginRes = AuthLoginResponse::fromJson(response);
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

                apiClient.setToken(token);
                api::AuthSession::instance().login(token, respUsername, role, userId, forcePasswordChange);

                if (forcePasswordChange)
                {
                    showInfo("Password change is required on first login.");
                    ChangePasswordScreen changePasswordScreen;
                    changePasswordScreen.show(apiClient);
                    showSuccess("Password updated. You are now logged in.");
                }
                else
                {
                    showSuccess("Login successful.");
                }
                break;
            }
            catch (const std::exception& ex)
            {
                showError(std::string("Login failed: ") + ex.what());
                ScreenUtils::readLine("Press Enter to continue");
                displayMenu();
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