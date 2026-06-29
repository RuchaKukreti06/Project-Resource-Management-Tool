#include "admin/ManageUsersScreen.h"
#include "api/ISessionStore.h"
#include "api/ApiException.h"
#include "utils/ConsoleInput.h"
#include "services/UserClientService.h"
#include "screens/ScreenUtils.h"
#include <iomanip>
#include <iostream>

using namespace AdminConstants;
using namespace AdminConstants::Users;

ManageUsersScreen::ManageUsersScreen(UserClientService& userService, api::ISessionStore& sessionStore)
    : userService_(userService), sessionStore_(sessionStore)
{
}

void ManageUsersScreen::displayMenu()
{
    decorator().render();
    std::cout << OPT_CREATE_USER << ". Create User Account\n";
    std::cout << OPT_VIEW_USERS << ". View All Users\n";
    std::cout << OPT_RESET_PWD << ". Reset User Password\n";
    std::cout << OPT_DEACTIVATE << ". Deactivate User\n";
    std::cout << OPT_BACK << ". Back\n";
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
    std::string choice = ConsoleInput::readLine("Enter option");
    if (choice == OPT_CREATE_USER)
    {
        createUser();
    }
    else if (choice == OPT_VIEW_USERS)
    {
        viewUsers();
    }
    else if (choice == OPT_RESET_PWD)
    {
        resetUserPassword();
    }
    else if (choice == OPT_DEACTIVATE)
    {
        deactivateUser();
    }
    else if (choice == OPT_BACK || ScreenUtils::equalsIgnoreCase(choice, "B"))
    {
        keepRunning_ = false;
    }
    else
    {
        showError("Invalid option. Please enter 1–5.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
}

std::string ManageUsersScreen::promptForValidEmail()
{
    std::string email;
    while (true)
    {
        email = ConsoleInput::readLine("Email");
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
    return email;
}

std::string ManageUsersScreen::promptForValidPassword()
{
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
    return tempPassword;
}

std::optional<std::string> ManageUsersScreen::promptForRole()
{
    std::cout << "Select Role:\n";
    std::cout << "  1. ADMIN\n";
    std::cout << "  2. MANAGER\n";
    std::cout << "  3. EMPLOYEE\n";
    std::string roleChoice = ConsoleInput::readLine("Choice (1-3)");
    
    if (roleChoice == "1") return ROLE_ADMIN;
    if (roleChoice == "2") return ROLE_MANAGER;
    if (roleChoice == "3") return ROLE_EMPLOYEE;
    return std::nullopt;
}

bool ManageUsersScreen::promptForBasicUserInfo(CreateUserRequest& req)
{
    req.fullName = ConsoleInput::readLine("Full Name");
    req.email = promptForValidEmail();
    
    req.username = ConsoleInput::readLine("Username");
    if (req.username.empty())
    {
        showError("Username cannot be empty.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return false;
    }
    return true;
}

void ManageUsersScreen::promptForCredentials(CreateUserRequest& req)
{
    req.password = promptForValidPassword();
    req.forcePasswordChange = true;
}

bool ManageUsersScreen::promptForRoleAndDepartmentDetails(CreateUserRequest& req)
{
    auto roleOpt = promptForRole();
    if (!roleOpt)
    {
        showError("Invalid role choice.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
        return false;
    }
    req.role = roleOpt.value();

    if (req.role == ROLE_MANAGER || req.role == ROLE_EMPLOYEE)
    {
        req.department = ConsoleInput::readLine("Department");
        req.designation = ConsoleInput::readLine("Designation");
    }
    return true;
}

std::optional<CreateUserRequest> ManageUsersScreen::promptForUserDetails()
{
    std::cout << "\n========== CREATE USER ACCOUNT ==========\n";
    
    CreateUserRequest req;
    
    if (!promptForBasicUserInfo(req)) return std::nullopt;
    promptForCredentials(req);
    if (!promptForRoleAndDepartmentDetails(req)) return std::nullopt;

    return req;
}

void ManageUsersScreen::createUser()
{
    try
    {
        auto createUserRequestOpt = promptForUserDetails();
        if (!createUserRequestOpt) return;

        auto response = userService_.createUser(createUserRequestOpt.value());
        if (!response.success)
        {
            showError(response.message);
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        showSuccess("Account created. User must change password on first login.");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

void ManageUsersScreen::handleReactivation()
{
    std::cout << "[R] Reactivate a user      [B] Back\n";
    std::string act = ConsoleInput::readLine("Enter choice");
    if (ScreenUtils::equalsIgnoreCase(act, "R"))
    {
        std::string targetUserId = ConsoleInput::readLine("Enter User ID to reactivate");
        auto reactResponse = userService_.reactivateUser(targetUserId);
        if (reactResponse.success)
        {
            showSuccess("Account reactivated. ✓");
        }
        else
        {
            showError(reactResponse.message);
        }
        ConsoleInput::waitForEnter("Press Enter to continue\n");
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
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        auto users = response.data;
        displayUsers(users);
        handleReactivation();
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue\n");
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
        auto targetUserIdOpt = promptForTargetUserId("Enter Username or User ID");
        if (!targetUserIdOpt) return;

        std::string newTempPwd = promptForValidPassword();

        auto response = userService_.resetPassword(targetUserIdOpt.value(), newTempPwd, true);

        if (!response.success)
        {
            showError(response.message);
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        showSuccess("Password reset. User will be prompted to change it on next login. ✓");
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue\n");
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
        auto targetUserIdOpt = promptForTargetUserId("Enter Username or User ID to deactivate");
        if (!targetUserIdOpt) return;
        
        std::string targetUserId = targetUserIdOpt.value();

        // Prevent admin from deactivating their own account
        int currentUserId = sessionStore_.userId();
        if (ScreenUtils::safeParseInt(targetUserId).value_or(-1) == currentUserId)
        {
            showError("You cannot deactivate your own account.");
            ConsoleInput::waitForEnter("Press Enter to continue\n");
            return;
        }

        std::cout << "\nAre you sure you want to deactivate this account?\n";
        std::cout << "[Y] Yes, Deactivate      [B] Back\n";
        std::string choice = ConsoleInput::readLine("Choice");
        if (ScreenUtils::equalsIgnoreCase(choice, "Y"))
        {
            auto response = userService_.deactivateUser(targetUserId);
            if (!response.success)
            {
                showError(response.message);
                ConsoleInput::waitForEnter("Press Enter to continue\n");
                return;
            }
            showSuccess("User deactivated. ✓");
        }
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const ApiException& ex)
    {
        showError(ex.what());
        ConsoleInput::waitForEnter("Press Enter to continue\n");
    }
    catch (const std::exception&)
    {
        showError("Something went wrong. Please try again.");
    }
}

ScreenDecorator ManageUsersScreen::decorator() const
{
    return ScreenDecorator("MANAGE USERS").withWidth(DEFAULT_PANEL_WIDTH).withPadding(DEFAULT_PADDING);
}

void ManageUsersScreen::displayUsers(const std::vector<UserDTO>& users)
{
    std::cout << "\n================ ALL USERS ================\n";
    std::cout << std::left
              << std::setw(ID_COLUMN_WIDTH) << "ID"
              << std::setw(USERNAME_COLUMN_WIDTH) << "Username"
              << std::setw(ROLE_COLUMN_WIDTH) << "Role"
              << std::setw(STATUS_COLUMN_WIDTH) << "Status" << "\n";
    ScreenUtils::printDivider();

    int activeCount = 0;
    int inactiveCount = 0;

    for (const auto& user : users)
    {
        std::cout << std::left
                  << std::setw(ID_COLUMN_WIDTH) << user.id
                  << std::setw(USERNAME_COLUMN_WIDTH) << ScreenUtils::truncate(ScreenUtils::valueOrDash(user.username), USERNAME_COLUMN_WIDTH - 1)
                  << std::setw(ROLE_COLUMN_WIDTH) << ScreenUtils::valueOrDash(user.role)
                  << std::setw(STATUS_COLUMN_WIDTH) << ScreenUtils::valueOrDash(user.status) << "\n";
        
        if (ScreenUtils::equalsIgnoreCase(user.status, STATUS_ACTIVE))
            activeCount++;
        else
            inactiveCount++;
    }
    ScreenUtils::printDivider();
    std::cout << "Total: " << users.size() << "  |  Active: " << activeCount
              << "  |  Inactive: " << inactiveCount << "\n\n";
}

std::optional<std::string> ManageUsersScreen::promptForTargetUserId(const std::string& prompt)
{
    std::string identifier = ConsoleInput::readLine(prompt);
    if (identifier.empty()) return std::nullopt;

    std::string userId = identifier;
    auto usersResponse = userService_.viewUsers();
    if (usersResponse.success)
    {
        for (const auto& u : usersResponse.data)
        {
            if (u.username == identifier || std::to_string(u.id) == identifier)
            {
                userId = std::to_string(u.id);
                std::cout << "\nUser found: " << u.username << " (" << u.role << ")\n";
                return userId;
            }
        }
    }
    
    return userId;
}
