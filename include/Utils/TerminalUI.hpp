#pragma once

#include <string>
#include <vector>
#include "Models/Room.hpp"
#include "Models/Guest.hpp"
#include "Models/Reservation.hpp"
#include "Models/ServiceItem.hpp"
#include "Services/HotelManager.hpp"

namespace hotel::ui {

class TerminalUI {
public:
    // Color definitions
    static const std::string RESET;
    static const std::string BOLD;
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string WHITE;
    static const std::string GRAY;

    // Visual elements
    static void printHeader(const std::string& title);
    static void printBanner();
    static void printSuccess(const std::string& message);
    static void printError(const std::string& message);
    static void printWarning(const std::string& message);
    static void printInfo(const std::string& message);
    static void pause();

    // Tables
    static void displayRoomsTable(const std::vector<const models::Room*>& rooms);
    static void displayGuestsTable(const std::vector<models::Guest>& guests);
    static void displayReservationsTable(const std::vector<models::Reservation>& reservations,
                                        const services::HotelManager& manager);
    static void displayServicesCatalog(const std::vector<models::ServiceItem>& services);
    static void displayAnalyticsDashboard(const services::AnalyticsReport& report);

    // Input Helpers
    static std::string readLine(const std::string& prompt);
    static int readInt(const std::string& prompt, int minVal, int maxVal);
    static double readDouble(const std::string& prompt, double minVal, double maxVal);
    static std::string readDate(const std::string& prompt, const std::string& defaultVal = "");
    static bool readConfirmation(const std::string& prompt, bool defaultYes = true);
};

} // namespace hotel::ui
