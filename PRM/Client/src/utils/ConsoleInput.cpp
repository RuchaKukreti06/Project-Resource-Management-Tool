#include "utils/ConsoleInput.h"
#include <iostream>
#include <limits>

namespace ConsoleInput
{

std::string readLine(const std::string& prompt)
{
    if (!prompt.empty())
    {
        std::cout << prompt << ": ";
    }
    std::string value;
    std::getline(std::cin, value);
    return value;
}

void waitForEnter(const std::string& message)
{
    if (!message.empty())
    {
        std::cout << message;
    }
    std::string dummy;
    std::getline(std::cin, dummy);
}

std::optional<int> readInt(const std::string& prompt)
{
    std::string input = readLine(prompt);
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

void clearBuffer()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

}
