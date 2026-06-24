#include "ChangePasswordScreen.h"
#include "api/ISessionStore.h"
#include "dto/ApiResponse.h"
#include "api/ApiException.h"

ChangePasswordScreen::ChangePasswordScreen(AuthClientService& authService, api::ISessionStore& sessionStore)
    : authService_(authService), sessionStore_(sessionStore)
{
}

ScreenDecorator ChangePasswordScreen::decorator() const
{
    return ScreenDecorator("CHANGE PASSWORD", "You must set a new password to continue.");
}

void ChangePasswordScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
}

void ChangePasswordScreen::show()
{
    displayMenu();
    handleInput();
}

void ChangePasswordScreen::handleInput()
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
            auto res = authService_.changePassword(sessionStore_.userId(), newPassword);

            if (!res.success)
            {
                showError(res.message);
                continue;
            }

            std::cout << "\n  Password updated. Welcome!\n\n";
            sessionStore_.login(sessionStore_.token(),
                                sessionStore_.username(),
                                sessionStore_.role(),
                                sessionStore_.userId(),
                                false);
            break;
        }
        catch (const ApiException& ex)
        {
            showError(ex.what());
        }
        catch (const std::exception&)
        {
            showError("Something went wrong. Please try again.");
        }
    }
}
