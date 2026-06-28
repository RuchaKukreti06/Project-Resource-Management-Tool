#include "utils/DateUtils.h"
#include <regex>
#include <ctime>
#include <chrono>

namespace DateUtils
{
    bool isValidDateFormatDDMMYYYY(const std::string& input)
    {
        std::regex pattern("^\\d{2}-\\d{2}-\\d{4}$");
        return std::regex_match(input, pattern);
    }

    bool isLeapYear(int year)
    {
        if (year % 4 != 0) return false;
        if (year % 100 == 0 && year % 400 != 0) return false;
        return true;
    }

    bool isRealDate(int day, int month, int year)
    {
        if (month < 1 || month > 12) return false;
        if (day < 1) return false;

        int daysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        if (month == 2 && isLeapYear(year))
        {
            daysInMonth[2] = 29;
        }

        return day <= daysInMonth[month];
    }

    std::string convertDDMMYYYYToYYYYMMDD(const std::string& input)
    {
        if (!isValidDateFormatDDMMYYYY(input)) return "";
        return input.substr(6, 4) + "-" + input.substr(3, 2) + "-" + input.substr(0, 2);
    }

    bool isFutureDate(const std::string& input)
    {
        if (!isValidDateFormatDDMMYYYY(input)) return false;
        
        int day = std::stoi(input.substr(0, 2));
        int month = std::stoi(input.substr(3, 2));
        int year = std::stoi(input.substr(6, 4));

        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* parts = std::localtime(&now_c);

        int currentYear = 1900 + parts->tm_year;
        int currentMonth = 1 + parts->tm_mon;
        int currentDay = parts->tm_mday;

        if (year > currentYear) return true;
        if (year == currentYear && month > currentMonth) return true;
        if (year == currentYear && month == currentMonth && day > currentDay) return true;

        return false;
    }

    bool isBeforeAllowedRange(const std::string& input)
    {
        if (!isValidDateFormatDDMMYYYY(input)) return false;
        int year = std::stoi(input.substr(6, 4));
        // Lower bound year 2015
        return year < 2015;
    }

    std::string validateDateDDMMYYYY(const std::string& input)
    {
        if (!isValidDateFormatDDMMYYYY(input))
        {
            return "Invalid date format. Use DD-MM-YYYY.";
        }

        int day = std::stoi(input.substr(0, 2));
        int month = std::stoi(input.substr(3, 2));
        int year = std::stoi(input.substr(6, 4));

        if (!isRealDate(day, month, year))
        {
            return "Invalid calendar date.";
        }

        if (isBeforeAllowedRange(input))
        {
            return "Date is too far in the past. Must be 2015 or later.";
        }

        if (isFutureDate(input))
        {
            return "Date cannot be in the future. Please enter a valid week date.";
        }

        return ""; // Valid
    }

    bool isValidDateFormatYYYYMMDD(const std::string& input)
    {
        std::regex pattern("^\\d{4}-\\d{2}-\\d{2}$");
        return std::regex_match(input, pattern);
    }

    bool isBeforeAllowedRangeYYYYMMDD(const std::string& input)
    {
        if (!isValidDateFormatYYYYMMDD(input)) return false;
        int year = std::stoi(input.substr(0, 4));
        // Lower bound year 2015
        return year < 2015;
    }

    std::string validateDateYYYYMMDD(const std::string& input, bool allowFuture)
    {
        if (!isValidDateFormatYYYYMMDD(input))
        {
            return "Invalid date format. Use YYYY-MM-DD.";
        }

        int year = std::stoi(input.substr(0, 4));
        int month = std::stoi(input.substr(5, 2));
        int day = std::stoi(input.substr(8, 2));

        if (!isRealDate(day, month, year))
        {
            return "Invalid calendar date.";
        }

        if (isBeforeAllowedRangeYYYYMMDD(input))
        {
            return "Date is too far in the past. Must be 2015 or later.";
        }

        if (!allowFuture)
        {
            // Convert to DD-MM-YYYY to reuse isFutureDate
            std::string dd_mm_yyyy = input.substr(8, 2) + "-" + input.substr(5, 2) + "-" + input.substr(0, 4);
            if (isFutureDate(dd_mm_yyyy))
            {
                return "Date cannot be in the future.";
            }
        }

        return ""; // Valid
    }

    std::string getCurrentDateYYYYMMDD()
    {
        std::time_t now = std::time(nullptr);
        std::tm local = {};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        local = *std::localtime(&now);
#endif
        char buffer[11] = {0};
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local);
        return std::string(buffer);
    }

    std::string getDateNextWeekYYYYMMDD()
    {
        std::time_t now = std::time(nullptr);
        std::time_t nextWeek = now + (7 * 24 * 60 * 60);
        std::tm localNext = {};
#ifdef _WIN32
        localtime_s(&localNext, &nextWeek);
#else
        localNext = *std::localtime(&nextWeek);
#endif
        char buffer[11] = {0};
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &localNext);
        return std::string(buffer);
    }
}
