#include "ChangePasswordScreen.h"
#include "api/ISessionStore.h"
#include "dto/ApiResponse.h"
#include "api/ApiException.h"
#include "utils/ConsoleInput.h"
#include "utils/Constants.h"

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
        auto [newPass, confirmPass] = promptPasswords();
        std::string choice = promptSaveOption();

        if (!validateChoice(choice)) continue;
        if (!validatePasswords(newPass, confirmPass)) continue;
        if (executePasswordChange(newPass)) break;
    }
}

std::pair<std::string, std::string> ChangePasswordScreen::promptPasswords()
{
    std::string newPassword     = ScreenUtils::readPassword(ui_constants::PROMPT_NEW_PASSWORD);
    std::string confirmPassword = ScreenUtils::readPassword(ui_constants::PROMPT_CONFIRM_PASSWORD);
    return {newPassword, confirmPassword};
}

std::string ChangePasswordScreen::promptSaveOption()
{
    std::cout << "\n";
    ScreenUtils::printDivider();
    std::cout << ui_constants::MSG_SAVE_OPTION;
    return ConsoleInput::readLine(ui_constants::PROMPT_ENTER_OPTION);
}

bool ChangePasswordScreen::validateChoice(const std::string& choice)
{
    if (choice != ui_constants::OPT_SAVE && choice != ui_constants::OPT_SAVE_LOWER)
    {
        showError(ui_constants::MSG_ERR_SAVE_OPTION);
        return false;
    }
    return true;
}

bool ChangePasswordScreen::validatePasswords(const std::string& newPassword, const std::string& confirmPassword)
{
    if (newPassword != confirmPassword)
    {
        showError(ui_constants::MSG_ERR_PASSWORDS_MISMATCH);
        return false;
    }

    std::string errorMsg;
    if (!ScreenUtils::isValidPassword(newPassword, errorMsg))
    {
        showError(errorMsg + ui_constants::MSG_ERR_TRY_AGAIN);
        return false;
    }
    return true;
}

bool ChangePasswordScreen::executePasswordChange(const std::string& newPassword)
{
    try
    {
        auto res = authService_.changePassword(sessionStore_.userId(), newPassword);

        if (!res.success)
        {
            return handleFailedChange(res.message);
        }

        std::cout << ui_constants::MSG_PASSWORD_UPDATED;
        updateSessionContext();
        return true;
    }
    catch (const ApiException& ex)
    {
        return handleChangeException(ex.what());
    }
    catch (const std::exception&)
    {
        return handleChangeException(ui_constants::MSG_ERR_UNKNOWN);
    }
}

bool ChangePasswordScreen::handleFailedChange(const std::string& message)
{
    showError(message);
    return false;
}

void ChangePasswordScreen::updateSessionContext()
{
    api::SessionData sessionData{
        sessionStore_.token(),
        sessionStore_.username(),
        sessionStore_.role(),
        sessionStore_.userId(),
        false // password changed
    };
    sessionStore_.login(sessionData);
}

bool ChangePasswordScreen::handleChangeException(const std::string& errorMessage)
{
    showError(errorMessage);
    return false;
}
