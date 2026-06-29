#include "LoginScreen.h"

#include "Router.h"
#include "dto/AuthDTO.h"
#include "api/ApiException.h"
#include "services/AuthClientService.h"
#include "services/AuthClientService.h"
#include "utils/ConsoleInput.h"
#include "utils/Constants.h"

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
    decorator().render();
    std::cout << "1. Login\n";
    std::cout << "2. Exit\n";
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
        std::string choice = ConsoleInput::readLine(ui_constants::PROMPT_ENTER_OPTION);
        if (choice == ui_constants::OPT_LOGIN)
        {
            if (processLoginOption())
            {
                break;
            }
        }
        else if (choice == ui_constants::OPT_EXIT)
        {
            handleExitOption();
        }
        else
        {
            showError(ui_constants::MSG_ERR_INVALID_OPTION);
            ConsoleInput::waitForEnter(ui_constants::MSG_PRESS_ENTER);
            displayMenu();
        }
    }
}

bool LoginScreen::processLoginOption()
{
    auto credentials = promptCredentials();
    if (!validateCredentials(credentials.first, credentials.second))
    {
        return false;
    }
    return executeLogin(credentials.first, credentials.second);
}

std::pair<std::string, std::string> LoginScreen::promptCredentials()
{
    std::string username = ConsoleInput::readLine(ui_constants::PROMPT_USERNAME);
    std::string password = ScreenUtils::readPassword(ui_constants::PROMPT_PASSWORD);
    return {username, password};
}

bool LoginScreen::validateCredentials(const std::string& username, const std::string& password)
{
    if (username.empty())
    {
        showError(ui_constants::MSG_ERR_USERNAME_EMPTY);
        ConsoleInput::waitForEnter(ui_constants::MSG_PRESS_ENTER);
        displayMenu();
        return false;
    }
    if (password.empty())
    {
        showError(ui_constants::MSG_ERR_PASSWORD_EMPTY);
        ConsoleInput::waitForEnter(ui_constants::MSG_PRESS_ENTER);
        displayMenu();
        return false;
    }
    return true;
}

bool LoginScreen::executeLogin(const std::string& username, const std::string& password)
{
    try
    {
        auto loginRes = authService_.login(username, password);
        if (!loginRes.success)
        {
            return handleFailedLogin(loginRes.message);
        }

        updateSessionContext(loginRes);
        displayLoginResult(loginRes.user.forcePasswordChange);
        return true;
    }
    catch (const ApiException& ex)
    {
        return handleLoginException(ex.what());
    }
    catch (const std::exception&)
    {
        return handleLoginException(ui_constants::MSG_ERR_UNKNOWN);
    }
}

bool LoginScreen::handleFailedLogin(const std::string& message)
{
    showError(message);
    ConsoleInput::waitForEnter(ui_constants::MSG_PRESS_ENTER);
    displayMenu();
    return false;
}

void LoginScreen::updateSessionContext(const AuthLoginResponse& loginRes)
{
    apiClient_.setToken(loginRes.token);
    
    api::SessionData sessionData{
        loginRes.token, 
        loginRes.user.username, 
        loginRes.user.role, 
        loginRes.user.id, 
        loginRes.user.forcePasswordChange
    };
    sessionStore_.login(sessionData);
}

void LoginScreen::displayLoginResult(bool forcePasswordChange)
{
    if (forcePasswordChange)
    {
        showInfo(ui_constants::MSG_PASSWORD_CHANGE_REQD);
    }
    else
    {
        showSuccess(ui_constants::MSG_LOGIN_SUCCESS);
    }
}

bool LoginScreen::handleLoginException(const std::string& errorMessage)
{
    showError(errorMessage);
    ConsoleInput::waitForEnter(ui_constants::MSG_PRESS_ENTER);
    displayMenu();
    return false;
}

void LoginScreen::handleExitOption()
{
    std::cout << ui_constants::MSG_GOODBYE;
    std::exit(0);
}
