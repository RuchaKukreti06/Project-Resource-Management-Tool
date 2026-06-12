#include "ChangePasswordScreen.h"
#include "AuthSession.h"

ScreenDecorator ChangePasswordScreen::decorator() const
{
    return ScreenDecorator("CHANGE PASSWORD", "You must set a new password to continue.");
}

void ChangePasswordScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
}

void ChangePasswordScreen::show(ApiClient& apiClient)
{
    displayMenu();
    handleInput(apiClient);
}

void ChangePasswordScreen::handleInput(ApiClient& apiClient)
{
    while (true)
    {
        std::string newPassword     = ScreenUtils::readPassword("New Password    ");
        std::string confirmPassword = ScreenUtils::readPassword("Confirm Password");

        std::cout << "\n";
        ScreenUtils::printDivider();
        std::cout << "[S] Save and Continue\n\n";

        std::string choice = ScreenUtils::readLine("Enter option");

        if (choice != "S" && choice != "s")
        {
            showError("Invalid option. Please enter S to save.");
            continue;
        }

        if (newPassword != confirmPassword)
        {
            showError("Passwords do not match. Please try again.");
            continue;
        }

        std::string errorMsg;
        if (!ScreenUtils::isValidPassword(newPassword, errorMsg))
        {
            showError(errorMsg + " Please try again.");
            continue;
        }

        try
        {
            auto response = apiClient.post(
                "/auth/change-password",
                {{"user_id", api::AuthSession::instance().userId()}, {"new_password", newPassword}});

            if (!response["success"].get<bool>())
            {
                showError(response["message"].get<std::string>());
                continue;
            }

            std::cout << "\n  Password updated. Welcome!\n\n";
            api::AuthSession::instance().login(api::AuthSession::instance().token(),
                                               api::AuthSession::instance().username(),
                                               api::AuthSession::instance().role(),
                                               api::AuthSession::instance().userId(),
                                               false);
            break;
        }
        catch (const std::exception& ex)
        {
            showError(ex.what());
        }
    }
}
