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
}

#endif
