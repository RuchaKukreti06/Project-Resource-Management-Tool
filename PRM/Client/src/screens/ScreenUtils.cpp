#include "ScreenUtils.h"
#include "utils/DateUtils.h"
#include <cctype>
#include <regex>
#include <algorithm>

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
    std::getline(std::cin, value);
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
    std::getline(std::cin, password);
    SetConsoleMode(hStdin, mode);
    std::cout << '\n';
#else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin, password);
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
    bool hasSpecial = false;
    for (char c : password)
    {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        if (std::isdigit(static_cast<unsigned char>(c))) hasNumber = true;
        if (!std::isalnum(static_cast<unsigned char>(c))) hasSpecial = true;
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
    if (!hasSpecial)
    {
        errorMsg = "Password must contain at least one special character.";
        return false;
    }
    return true;
}

bool isValidEmail(const std::string& email)
{
    const std::regex pattern(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    return std::regex_match(email, pattern);
}

std::optional<int> safeParseInt(const std::string& input)
{
    if (input.empty()) return std::nullopt;
    try
    {
        size_t pos;
        int val = std::stoi(input, &pos);
        if (pos != input.length()) return std::nullopt;
        return val;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

std::optional<double> safeParseDouble(const std::string& input)
{
    if (input.empty()) return std::nullopt;
    try
    {
        size_t pos;
        double val = std::stod(input, &pos);
        if (pos != input.length()) return std::nullopt;
        return val;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

std::string valueOrDash(const std::string& value)
{
    return value.empty() ? "-" : value;
}

std::string truncate(const std::string& value, size_t maxLength)
{
    if (value.length() <= maxLength) return value;
    if (maxLength < 4) return value.substr(0, maxLength);
    return value.substr(0, maxLength - 3) + "...";
}

std::string trim(const std::string& s)
{
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

std::string toLower(const std::string& s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool equalsIgnoreCase(const std::string& a, const std::string& b)
{
    return toLower(trim(a)) == toLower(trim(b));
}

std::string readValidDate(const std::string& label, bool allowFuture, const std::string& fallback)
{
    std::string dateStr;
    while (true)
    {
        std::string fullPrompt = label + (fallback.empty() ? "" : (" (" + fallback + ")"));
        dateStr = readLine(fullPrompt);
        
        if (dateStr.empty()) return fallback;

        std::string err = DateUtils::validateDateYYYYMMDD(dateStr, allowFuture);
        if (!err.empty())
        {
            std::cout << "\n  Date error: " << err << "\n\n";
            continue;
        }
        break;
    }
    return dateStr;
}

std::string readOptionalDateForUpdate(const std::string& label, bool allowFuture, const std::string& fallback)
{
    std::string dateStr;
    while (true)
    {
        std::string fullPrompt = label;
        if (fallback.empty())
        {
            fullPrompt += " (Press Enter to skip)";
        }
        else
        {
            fullPrompt += " (Press Enter to keep: " + fallback + ")";
        }
        
        dateStr = readLine(fullPrompt);
        
        if (dateStr.empty()) return fallback;

        std::string err = DateUtils::validateDateYYYYMMDD(dateStr, allowFuture);
        if (!err.empty())
        {
            std::cout << "\n  Date error: " << err << "\n\n";
            continue;
        }
        break;
    }
    return dateStr;
}

std::optional<std::string> promptForDate(const std::string& label, bool required)
{
    std::string dateStr = readLine(label + " (YYYY-MM-DD)" + (required ? "" : " [Optional]"));
    if (dateStr.empty())
    {
        return required ? std::nullopt : std::make_optional("");
    }
    std::string error = DateUtils::validateDateYYYYMMDD(dateStr, true);
    if (!error.empty())
    {
        std::cout << "\nError: " << error << "\n";
        return std::nullopt;
    }
    return dateStr;
}

std::optional<int> promptForIntBounds(const std::string& label, int min, int max, const std::string& errorMsg)
{
    std::string input = readLine(label);
    auto parsed = safeParseInt(input);
    if (!parsed || parsed.value() < min || parsed.value() > max)
    {
        std::cout << "\nError: " << errorMsg << "\n";
        return std::nullopt;
    }
    return parsed.value();
}

std::optional<int> promptForInt(const std::string& label, const std::string& errorMsg)
{
    std::string input = readLine(label);
    auto parsed = safeParseInt(input);
    if (!parsed)
    {
        std::cout << "\nError: " << errorMsg << "\n";
        return std::nullopt;
    }
    return parsed.value();
}

}
