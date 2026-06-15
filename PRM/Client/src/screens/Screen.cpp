#include "Screen.h"

#include <iostream>

void Screen::clearScreen() const
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void Screen::printDivider() const
{
    std::cout << "----------------------------------------\n";
}

void Screen::showError(const std::string& message)
{
    std::cout << "\n[ERROR] " << message << "\n\n";
}

void Screen::showSuccess(const std::string& message)
{
    std::cout << "\n[SUCCESS] " << message << "\n\n";
}

void Screen::showInfo(const std::string& message)
{
    std::cout << "\n[INFO] " << message << "\n\n";
}

void Screen::showWarning(const std::string& message)
{
    std::cout << "\n[WARNING] " << message << "\n\n";
}