#ifndef CLIENT_CONSTANTS_H
#define CLIENT_CONSTANTS_H

#include <string>

namespace constants {
    const std::string BASE_URL = "http://127.0.0.1:8080";

    constexpr int CONNECTION_TIMEOUT_SEC = 5;
    constexpr int READ_TIMEOUT_SEC = 60;

    constexpr int HTTP_STATUS_OK_MIN = 200;
    constexpr int HTTP_STATUS_OK_MAX = 299;
    
    constexpr int HTTP_STATUS_BAD_REQUEST = 400;
    constexpr int HTTP_STATUS_UNAUTHORIZED = 401;
    constexpr int HTTP_STATUS_FORBIDDEN = 403;
    constexpr int HTTP_STATUS_NOT_FOUND = 404;
    constexpr int HTTP_STATUS_INTERNAL_SERVER_ERROR = 500;
}

namespace ui_constants {
    const std::string PROMPT_ENTER_OPTION = "Enter option";
    const std::string PROMPT_USERNAME = "Username";
    const std::string PROMPT_PASSWORD = "Password";
    const std::string MSG_ERR_USERNAME_EMPTY = "Username cannot be empty. Please try again.";
    const std::string MSG_ERR_PASSWORD_EMPTY = "Password cannot be empty. Please try again.";
    const std::string MSG_ERR_INVALID_OPTION = "Please select a valid option.";
    const std::string MSG_ERR_UNKNOWN = "Something went wrong. Please try again.";
    const std::string MSG_PRESS_ENTER = "Press Enter to continue\n";
    const std::string MSG_LOGIN_SUCCESS = "Login successful.";
    const std::string MSG_PASSWORD_CHANGE_REQD = "Password change is required on first login.";
    const std::string MSG_GOODBYE = "Goodbye.\n";
    const std::string OPT_LOGIN = "1";
    const std::string OPT_EXIT = "2";

    const std::string PROMPT_NEW_PASSWORD = "New Password    ";
    const std::string PROMPT_CONFIRM_PASSWORD = "Confirm Password";
    const std::string MSG_SAVE_OPTION = "[S] Save and Continue\n\n";
    const std::string OPT_SAVE = "S";
    const std::string OPT_SAVE_LOWER = "s";
    const std::string MSG_ERR_SAVE_OPTION = "Invalid option. Please enter S to save.";
    const std::string MSG_ERR_PASSWORDS_MISMATCH = "Passwords do not match. Please try again.";
    const std::string MSG_ERR_TRY_AGAIN = " Please try again.";
    const std::string MSG_PASSWORD_UPDATED = "\n  Password updated. Welcome!\n\n";
}

#endif // CLIENT_CONSTANTS_H
