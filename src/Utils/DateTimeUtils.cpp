#include "Utils/DateTimeUtils.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>
#include <regex>

namespace hotel::utils {

std::string DateTimeUtils::getCurrentDate() {
    std::time_t now = std::time(nullptr);
    std::tm localTm{};
#if defined(_WIN32)
    localtime_s(&localTm, &now);
#else
    localtime_r(&now, &localTm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&localTm, "%Y-%m-%d");
    return oss.str();
}

std::string DateTimeUtils::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm localTm{};
#if defined(_WIN32)
    localtime_s(&localTm, &now);
#else
    localtime_r(&now, &localTm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool DateTimeUtils::isValidDateFormat(const std::string& dateStr) {
    static const std::regex pattern(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(dateStr, pattern)) {
        return false;
    }

    int y = 0, m = 0, d = 0;
    char dash1 = 0, dash2 = 0;
    std::istringstream iss(dateStr);
    if (!(iss >> y >> dash1 >> m >> dash2 >> d)) {
        return false;
    }

    if (m < 1 || m > 12 || d < 1 || d > 31) {
        return false;
    }

    std::tm tmVal{};
    tmVal.tm_year = y - 1900;
    tmVal.tm_mon = m - 1;
    tmVal.tm_mday = d;
    tmVal.tm_isdst = -1;

    std::time_t t = std::mktime(&tmVal);
    if (t == -1) return false;

    return (tmVal.tm_year == y - 1900 &&
            tmVal.tm_mon == m - 1 &&
            tmVal.tm_mday == d);
}

int DateTimeUtils::calculateDaysBetween(const std::string& startDate, const std::string& endDate) {
    if (!isValidDateFormat(startDate) || !isValidDateFormat(endDate)) {
        return 0;
    }

    int sy = 0, sm = 0, sd = 0;
    int ey = 0, em = 0, ed = 0;
    char dash;

    std::istringstream siss(startDate);
    siss >> sy >> dash >> sm >> dash >> sd;

    std::istringstream eiss(endDate);
    eiss >> ey >> dash >> em >> dash >> ed;

    std::tm startTm{};
    startTm.tm_year = sy - 1900;
    startTm.tm_mon = sm - 1;
    startTm.tm_mday = sd;
    startTm.tm_hour = 12;

    std::tm endTm{};
    endTm.tm_year = ey - 1900;
    endTm.tm_mon = em - 1;
    endTm.tm_mday = ed;
    endTm.tm_hour = 12;

    std::time_t sTime = std::mktime(&startTm);
    std::time_t eTime = std::mktime(&endTm);

    double diffSeconds = std::difftime(eTime, sTime);
    return static_cast<int>(diffSeconds / (24 * 3600));
}

std::string DateTimeUtils::addDaysToDate(const std::string& startDate, int days) {
    if (!isValidDateFormat(startDate)) {
        return startDate;
    }

    int y = 0, m = 0, d = 0;
    char dash;
    std::istringstream iss(startDate);
    iss >> y >> dash >> m >> dash >> d;

    std::tm tmVal{};
    tmVal.tm_year = y - 1900;
    tmVal.tm_mon = m - 1;
    tmVal.tm_mday = d + days;
    tmVal.tm_hour = 12;

    std::time_t t = std::mktime(&tmVal);
    if (t == -1) return startDate;

    std::ostringstream oss;
    oss << std::put_time(&tmVal, "%Y-%m-%d");
    return oss.str();
}

std::string DateTimeUtils::generateId(const std::string& prefix) {
    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> dist(1000, 9999);

    std::time_t now = std::time(nullptr);
    std::tm localTm{};
#if defined(_WIN32)
    localtime_s(&localTm, &now);
#else
    localtime_r(&now, &localTm);
#endif

    std::ostringstream oss;
    oss << prefix << "-"
        << std::put_time(&localTm, "%Y%m%d")
        << "-" << dist(rng);
    return oss.str();
}

} // namespace hotel::utils
