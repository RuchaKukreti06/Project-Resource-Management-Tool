#include "utils/ConsoleInput.h"
#include <iostream>
#include <limits>
#include <algorithm>

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
    // Sanitize input: remove non-printable control characters
    value.erase(std::remove_if(value.begin(), value.end(), [](unsigned char c) {
        return c < 32 && c != '\t';
    }), value.end());
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
