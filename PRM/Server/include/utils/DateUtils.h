#pragma once

#include <ctime>
#include <string>

namespace utils
{

inline std::string currentDateIso()
{
    std::time_t now = std::time(nullptr);
    std::tm local  = {};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    local = *std::localtime(&now);
#endif
    char buffer[11] = {0};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local);
    return buffer;
}

} // namespace utils
