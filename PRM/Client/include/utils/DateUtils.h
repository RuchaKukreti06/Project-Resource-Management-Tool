#pragma once

#include <string>

namespace DateUtils
{
    // Checks if the string format is exactly DD-MM-YYYY
    bool isValidDateFormatDDMMYYYY(const std::string& input);

    // Checks if the given year is a leap year
    bool isLeapYear(int year);

    // Validates whether the given day, month, and year form a real calendar date
    bool isRealDate(int day, int month, int year);

    // Converts DD-MM-YYYY to YYYY-MM-DD
    std::string convertDDMMYYYYToYYYYMMDD(const std::string& input);

    // Checks if the date string (DD-MM-YYYY) represents a date that is in the future
    bool isFutureDate(const std::string& input);

    // Checks if the date string (DD-MM-YYYY) represents a date that is before the lower bound (2020-01-01)
    bool isBeforeAllowedRange(const std::string& input);

    // Validates everything and returns an error message if invalid, or empty string if valid.
    std::string validateDateDDMMYYYY(const std::string& input);

    // Checks if the string format is exactly YYYY-MM-DD
    bool isValidDateFormatYYYYMMDD(const std::string& input);

    // Checks if the date string (YYYY-MM-DD) represents a date that is before the lower bound (2020-01-01)
    bool isBeforeAllowedRangeYYYYMMDD(const std::string& input);

    // Validates everything for YYYY-MM-DD and returns an error message if invalid, or empty string if valid.
    std::string validateDateYYYYMMDD(const std::string& input, bool allowFuture = true);

    // Returns the current date in YYYY-MM-DD format
    std::string getCurrentDateYYYYMMDD();

    // Returns the date 7 days from now in YYYY-MM-DD format
    std::string getDateNextWeekYYYYMMDD();
}
