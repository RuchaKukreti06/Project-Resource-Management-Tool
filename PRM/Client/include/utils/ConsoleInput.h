#pragma once

#include <string>
#include <optional>

namespace ConsoleInput
{
    // Read a full line using std::getline, ensuring leftover newlines are cleared.
    std::string readLine(const std::string& prompt);

    // Prompt the user to press Enter to continue. Uses std::getline.
    void waitForEnter(const std::string& message = "Press Enter to continue");

    // Read an integer using std::getline and std::stoi. Returns nullopt if invalid.
    std::optional<int> readInt(const std::string& prompt);

    // Clear any remaining input from std::cin (e.g. after raw std::cin >> extraction).
    void clearBuffer();
}
