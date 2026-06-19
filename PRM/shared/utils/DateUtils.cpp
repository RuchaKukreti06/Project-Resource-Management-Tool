#include "DateUtils.h"
#include <ctime>
namespace DateUtils
{
    std::string currentDateIso()
    {
        std::time_t now = std::time(nullptr);
        std::tm local = {};
        localtime_s(&local, &now);
        char buffer[11] = {0};
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local);
        return buffer;
    }
}
