#include "UserAccountCreationScreen.h"
#include "dto/ApiResponse.h"
#include "api/ApiException.h"
#include "utils/ConsoleInput.h"
#include "services/UserClientService.h"
void UserAccountCreationScreen::show(IApiClient& apiClient)
{
    Screen::show(apiClient);
}

void UserAccountCreationScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    std::cout << "Full Name        : ";
    std::cout << "Email            : ";
    std::cout << "Username         : ";
    std::cout << "Password         : ";
    std::cout << "Confirm Password : ";
}

void UserAccountCreationScreen::handleInput(IApiClient& apiClient)
{
    std::string fullName        = ConsoleInput::readLine("Full Name       ");
    std::string email           = ConsoleInput::readLine("Email           ");
    std::string username        = ConsoleInput::readLine("Username        ");
    std::string password        = ConsoleInput::readLine("Password        ");
    std::string confirmPassword = ConsoleInput::readLine("Confirm Password");

    std::cout << "\n";
    ScreenUtils::printDivider();
    std::cout << "[C] Create Account\n\n";

    std::string choice;
    std::cout << "Enter option: ";
    std::cin >> choice;

    if (choice != "C" && choice != "c")
    {
        showError("Invalid option.");
        handleInput(apiClient);
        return;
    }

    if (fullName.empty() || email.empty() || username.empty() || password.empty())
    {
        showError("All fields are required.");
        handleInput(apiClient);
        return;
    }

    if (password != confirmPassword)
    {
        showError("Passwords do not match. Please try again.");
        handleInput(apiClient);
        return;
    }

    try
    {
        UserClientService userService(apiClient);
        RegisterUserRequest req;
        req.username = username;
        req.password = password;
        req.email = email;
        req.fullName = fullName;

        auto res = userService.registerUser(req);
        if (!res.success)
        {
            showError(res.message);
            handleInput(apiClient);
            return;
        }

        std::cout << "\n  Account created successfully! Please log in. ✓\n\n";
    }
    catch (const ApiException& e)
    {
        showError(e.what());
        handleInput(apiClient);
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

ScreenDecorator UserAccountCreationScreen::decorator() const
{
    return ScreenDecorator("Create New Account").withWidth(40).withPadding(2);
}
