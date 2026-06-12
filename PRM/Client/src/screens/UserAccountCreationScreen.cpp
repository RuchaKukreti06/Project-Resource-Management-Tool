#include "UserAccountCreationScreen.h"

void UserAccountCreationScreen::show(ApiClient& apiClient)
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

void UserAccountCreationScreen::handleInput(ApiClient& apiClient)
{
    std::string fullName        = ScreenUtils::readLine("Full Name       ");
    std::string email           = ScreenUtils::readLine("Email           ");
    std::string username        = ScreenUtils::readLine("Username        ");
    std::string password        = ScreenUtils::readLine("Password        ");
    std::string confirmPassword = ScreenUtils::readLine("Confirm Password");

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
        auto response = apiClient.post("/auth/register",
                                       {{"username", username},
                                        {"password", password},
                                        {"email", email},
                                        {"full_name", fullName}});

        if (!response["success"].get<bool>())
        {
            showError(response["message"].get<std::string>());
            handleInput(apiClient);
            return;
        }

        std::cout << "\n  Account created successfully! Please log in. ✓\n\n";
    }
    catch (const std::exception& e)
    {
        showError(std::string("An error occurred: ") + e.what());
        handleInput(apiClient);
    }
}

ScreenDecorator UserAccountCreationScreen::decorator() const
{
    return ScreenDecorator("Create New Account").withWidth(40).withPadding(2);
}
