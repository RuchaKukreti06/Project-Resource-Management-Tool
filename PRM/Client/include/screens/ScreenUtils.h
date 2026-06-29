#ifndef SCREEN_UTILS_H
#define SCREEN_UTILS_H

#include <iostream>
#include <string>
#include <optional>

namespace ScreenUtils
{
void clearScreen();
void printDivider();
void printPrompt(const std::string& label);
std::string readLine(const std::string& label);
std::string readPassword(const std::string& label);
bool isValidPassword(const std::string& password, std::string& errorMsg);
bool isValidEmail(const std::string& email);
std::optional<int> safeParseInt(const std::string& input);
std::optional<double> safeParseDouble(const std::string& input);

std::string valueOrDash(const std::string& value);
std::string truncate(const std::string& value, size_t maxLength);

std::string trim(const std::string& s);
std::string toLower(const std::string& s);
bool equalsIgnoreCase(const std::string& a, const std::string& b);

std::string readValidDate(const std::string& label, bool allowFuture, const std::string& fallback = "");
std::string readOptionalDateForUpdate(const std::string& label, bool allowFuture, const std::string& fallback);
std::optional<std::string> promptForDate(const std::string& label, bool required);
std::optional<int> promptForIntBounds(const std::string& label, int min, int max, const std::string& errorMsg);
std::optional<int> promptForInt(const std::string& label, const std::string& errorMsg = "Invalid format.");
}

#endif
