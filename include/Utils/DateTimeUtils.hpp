#pragma once

#include <string>
#include <chrono>

namespace hotel::utils {

class DateTimeUtils {
public:
    // Returns current local date in "YYYY-MM-DD" format
    static std::string getCurrentDate();

    // Returns current date and time in "YYYY-MM-DD HH:MM:SS" format
    static std::string getCurrentTimestamp();

    // Validates if string follows YYYY-MM-DD format and is a valid calendar date
    static bool isValidDateFormat(const std::string& dateStr);

    // Calculates difference in days between two YYYY-MM-DD dates (end - start)
    static int calculateDaysBetween(const std::string& startDate, const std::string& endDate);

    // Returns a future date offset by given number of days from startDate
    static std::string addDaysToDate(const std::string& startDate, int days);

    // Generate a unique identifier with a given prefix (e.g., RES-20261107-1234)
    static std::string generateId(const std::string& prefix);
};

} // namespace hotel::utils
