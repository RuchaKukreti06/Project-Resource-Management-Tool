#include "admin/ManageUsersScreen.h"
#include "AuthSession.h"
#include "dto/ApiResponse.h"
#include "dto/UserDTO.h"
#include "api/ISessionStore.h"
#include "api/ApiException.h"
#include "services/UserClientService.h"

ManageUsersScreen::ManageUsersScreen(UserClientService& userService, api::ISessionStore& sessionStore)
    : userService_(userService), sessionStore_(sessionStore)
{
}

void ManageUsersScreen::displayMenu()
{
    // clearScreen();
    decorator().render();
    std::cout << "1. Create User Account\n";
    std::cout << "2. View All Users\n";
    std::cout << "3. Reset User Password\n";
    std::cout << "4. Deactivate User\n";
    std::cout << "5. Back\n";
}

void ManageUsersScreen::show()
{
    keepRunning_ = true;
    while (keepRunning_)
    {
        displayMenu();
        handleInput();
    }
}

void ManageUsersScreen::handleInput()
{
    std::string choice = ScreenUtils::readLine("Enter option");
    if (choice == "1")
    {
        createUser();
    }
    else if (choice == "2")
    {
        viewUsers();
    }
    else if (choice == "3")
    {
        resetUserPassword();
    }
    else if (choice == "4")
    {
        deactivateUser();
    }
    else if (choice == "5" || choice == "B" || choice == "b")
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1–5.");
        ScreenUtils::readLine("Press Enter to continue");
    }
}

void ManageUsersScreen::createUser()
{
    try
    {
        std::cout << "\n========== CREATE USER ACCOUNT ==========\n";
        std::string fullName = ScreenUtils::readLine("Full Name");
        std::string email;
        while (true)
        {
            email = ScreenUtils::readLine("Email");
            if (email.empty())
            {
                std::cout << "\n  Email is mandatory.\n\n";
                continue;
            }
            if (!ScreenUtils::isValidEmail(email))
            {
                std::cout << "\n  Invalid email format. Please try again.\n\n";
                continue;
            }
            break;
        }
        std::string username = ScreenUtils::readLine("Username");
        std::string tempPassword;
        while (true)
        {
            tempPassword = ScreenUtils::readPassword("Temporary Password");
            if (tempPassword.empty())
            {
                std::cout << "\n  Temporary Password is mandatory.\n\n";
                continue;
            }

            std::string errorMsg;
            if (!ScreenUtils::isValidPassword(tempPassword, errorMsg))
            {
                std::cout << "\n  " << errorMsg << "\n\n";
                continue;
            }
            break;
        }

        std::cout << "Select Role:\n";
        std::cout << "  1. ADMIN\n";
        std::cout << "  2. MANAGER\n";
        std::cout << "  3. EMPLOYEE\n";
        std::string roleChoice = ScreenUtils::readLine("Choice (1-3)");

        std::string role;
        if (roleChoice == "1")
            role = "ADMIN";
        else if (roleChoice == "2")
            role = "MANAGER";
        else if (roleChoice == "3")
            role = "EMPLOYEE";
        else
        {
            showError("Invalid role choice.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        std::string dept;
        std::string desg;
        if (role == "MANAGER" || role == "EMPLOYEE")
        {
            dept = ScreenUtils::readLine("Department");
            desg = ScreenUtils::readLine("Designation");
        }

        CreateUserRequest req;
        req.username = username;
        req.password = tempPassword;
        req.role = role;
        req.email = email;
        req.fullName = fullName;
        req.department = dept;
        req.designation = desg;
        req.forcePasswordChange = true;

        auto response = userService_.createUser(req);

        if (!response.success)
        {
            showError(response.message);
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        showSuccess("Account created. User must change password on first login.");
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

void ManageUsersScreen::viewUsers()
{
    try
    {
        auto response = userService_.viewUsers();
        if (!response.success)
        {
            showError(response.message);
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto users = response.data;
        // clearScreen();
        std::cout << "\n================ ALL USERS ================\n";
        std::cout << std::left << std::setw(6) << "ID" << std::setw(20) << "Username"
                  << std::setw(12) << "Role" << std::setw(12) << "Status" << "\n";
        ScreenUtils::printDivider();

        int activeCount = 0;
        int inactiveCount = 0;

        for (const auto& user : users)
        {
            std::cout << std::left << std::setw(6) << user.id << std::setw(20)
                      << user.username << std::setw(12)
                      << user.role << std::setw(12) << user.status << "\n";

            if (user.status == "ACTIVE" || user.status == "Active")
                activeCount++;
            else
                inactiveCount++;
        }
        ScreenUtils::printDivider();
        std::cout << "Total: " << users.size() << "  |  Active: " << activeCount
                  << "  |  Inactive: " << inactiveCount << "\n\n";

        std::cout << "[R] Reactivate a user      [B] Back\n";
        std::string act = ScreenUtils::readLine("Enter choice");
        if (act == "R" || act == "r")
        {
            std::string uId = ScreenUtils::readLine("Enter User ID to reactivate");
            auto reactResponse = userService_.reactivateUser(uId);
            if (reactResponse.success)
            {
                showSuccess("Account reactivated. ✓");
            }
            else
            {
                showError(reactResponse.message);
            }
            ScreenUtils::readLine("Press Enter to continue");
        }
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

void ManageUsersScreen::resetUserPassword()
{
    try
    {
        std::string identifier = ScreenUtils::readLine("Enter Username or User ID");

        // Find user to get ID if username was entered
        std::string userId = identifier;
        auto usersResponse = userService_.viewUsers();
        if (usersResponse.success)
        {
            for (const auto& u : usersResponse.data)
            {
                if (u.username == identifier || std::to_string(u.id) == identifier)
                {
                    userId = std::to_string(u.id);
                    std::cout << "\nUser found: " << u.username << " ("
                              << u.role << ")\n";
                    break;
                }
            }
        }

        std::string newTempPwd = ScreenUtils::readPassword("New Temporary Password");
        std::string errorMsg;
        if (!ScreenUtils::isValidPassword(newTempPwd, errorMsg))
        {
            showError("Temporary password complexity error: " + errorMsg);
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        auto response = userService_.resetPassword(userId, newTempPwd, true);

        if (!response.success)
        {
            showError(response.message);
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        showSuccess("Password reset. User will be prompted to change it on next login. ✓");
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

void ManageUsersScreen::deactivateUser()
{
    try
    {
        std::string identifier = ScreenUtils::readLine("Enter Username or User ID");

        // Find user to get ID if username was entered
        std::string userId = identifier;
        auto usersResponse = userService_.viewUsers();
        if (usersResponse.success)
        {
            for (const auto& u : usersResponse.data)
            {
                if (u.username == identifier || std::to_string(u.id) == identifier)
                {
                    userId = std::to_string(u.id);
                    std::cout << "\nUser found: " << u.username << " ("
                              << u.role << ")\n";
                    break;
                }
            }
        }

        // Prevent admin from deactivating their own account
        int currentUserId = sessionStore_.userId();
        if (!userId.empty() && ScreenUtils::safeParseInt(userId).value_or(-1) == currentUserId)
        {
            showError("You cannot deactivate your own account.");
            ScreenUtils::readLine("Press Enter to continue");
            return;
        }

        std::cout << "\nAre you sure you want to deactivate this account?\n";
        std::cout << "[Y] Yes, Deactivate      [B] Back\n";
        std::string choice = ScreenUtils::readLine("Choice");
        if (choice == "Y" || choice == "y")
        {
            auto response = userService_.deactivateUser(userId);
            if (!response.success)
            {
                showError(response.message);
                ScreenUtils::readLine("Press Enter to continue");
                return;
            }
            showSuccess("User deactivated. ✓");
        }
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ScreenUtils::readLine("Press Enter to continue");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

ScreenDecorator ManageUsersScreen::decorator() const
{
    return ScreenDecorator("MANAGE USERS").withWidth(40).withPadding(2);
}
