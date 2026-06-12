#include "ScreenUtils.h"

#include <cctype>
#include <regex>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace ScreenUtils
{
void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printDivider()
{
    std::cout << "──────────────────────────────────────────────\n";
}

void printPrompt(const std::string& label)
{
    std::cout << label << ": ";
}

std::string readLine(const std::string& label)
{
    std::string value;
    std::cout << label << ": ";
    std::getline(std::cin >> std::ws, value);
    return value;
}

std::string readPassword(const std::string& label)
{
    std::cout << label << ": ";
    std::string password;
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
    std::getline(std::cin >> std::ws, password);
    SetConsoleMode(hStdin, mode);
    std::cout << '\n';
#else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin >> std::ws, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << '\n';
#endif
    return password;
}

bool isValidPassword(const std::string& password, std::string& errorMsg)
{
    if (password.length() < 8)
    {
        errorMsg = "Password must be at least 8 characters long.";
        return false;
    }
    bool hasUpper = false;
    bool hasNumber = false;
    for (char c : password)
    {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        if (std::isdigit(static_cast<unsigned char>(c))) hasNumber = true;
    }
    if (!hasUpper)
    {
        errorMsg = "Password must contain at least one uppercase letter.";
        return false;
    }
    if (!hasNumber)
    {
        errorMsg = "Password must contain at least one number.";
        return false;
    }
    return true;
}

bool isValidEmail(const std::string& email)
{
    const std::regex pattern(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    return std::regex_match(email, pattern);
}
} 
