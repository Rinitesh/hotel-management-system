#include "Utils/TerminalUI.hpp"
#include "Utils/DateTimeUtils.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <algorithm>

namespace hotel::ui {

const std::string TerminalUI::RESET   = "\033[0m";
const std::string TerminalUI::BOLD    = "\033[1m";
const std::string TerminalUI::RED     = "\033[31m";
const std::string TerminalUI::GREEN   = "\033[32m";
const std::string TerminalUI::YELLOW  = "\033[33m";
const std::string TerminalUI::BLUE    = "\033[34m";
const std::string TerminalUI::MAGENTA = "\033[35m";
const std::string TerminalUI::CYAN    = "\033[36m";
const std::string TerminalUI::WHITE   = "\033[37m";
const std::string TerminalUI::GRAY    = "\033[90m";

void TerminalUI::printBanner() {
    std::cout << CYAN << BOLD;
    std::cout << R"(
  ┌───────────────────────────────────────────────────────────────┐
  │         ★  GRAND HORIZON LUXURY RESORT & HOTEL  ★             │
  │              Enterprise Property Management System            │
  └───────────────────────────────────────────────────────────────┘
)" << RESET << "\n";
}

void TerminalUI::printHeader(const std::string& title) {
    std::cout << "\n" << BOLD << CYAN << "=== " << title << " ===" << RESET << "\n\n";
}

void TerminalUI::printSuccess(const std::string& message) {
    std::cout << GREEN << BOLD << "✓ SUCCESS: " << RESET << GREEN << message << RESET << "\n";
}

void TerminalUI::printError(const std::string& message) {
    std::cout << RED << BOLD << "✗ ERROR: " << RESET << RED << message << RESET << "\n";
}

void TerminalUI::printWarning(const std::string& message) {
    std::cout << YELLOW << BOLD << "⚠ WARNING: " << RESET << YELLOW << message << RESET << "\n";
}

void TerminalUI::printInfo(const std::string& message) {
    std::cout << BLUE << "ℹ INFO: " << RESET << message << "\n";
}

void TerminalUI::pause() {
    std::cout << GRAY << "\nPress Enter to continue..." << RESET;
    std::string dummy;
    std::getline(std::cin, dummy);
}

void TerminalUI::displayRoomsTable(const std::vector<const models::Room*>& rooms) {
    if (rooms.empty()) {
        printWarning("No rooms found matching criteria.");
        return;
    }

    std::cout << BOLD
              << std::left
              << std::setw(8)  << "Room #"
              << std::setw(18) << "Category"
              << std::setw(10) << "Capacity"
              << std::setw(14) << "Base Rate/nt"
              << std::setw(16) << "Status"
              << "Key Amenities" << RESET << "\n";
    std::cout << std::string(90, '-') << "\n";

    for (const auto* room : rooms) {
        std::string statusColor = GREEN;
        if (room->getStatus() == models::RoomStatus::Occupied) statusColor = RED;
        else if (room->getStatus() == models::RoomStatus::Reserved) statusColor = YELLOW;
        else if (room->getStatus() == models::RoomStatus::Maintenance) statusColor = MAGENTA;

        std::string amenitiesStr;
        const auto& am = room->getAmenities();
        for (size_t i = 0; i < am.size() && i < 2; ++i) {
            if (i > 0) amenitiesStr += ", ";
            amenitiesStr += am[i];
        }
        if (am.size() > 2) amenitiesStr += ", ...";

        std::cout << std::left
                  << std::setw(8)  << room->getRoomNumber()
                  << std::setw(18) << room->getTypeName()
                  << std::setw(10) << (std::to_string(room->getCapacity()) + " guests")
                  << std::setw(14) << ("$" + std::to_string(static_cast<int>(room->getBaseRate())) + ".00")
                  << statusColor << std::setw(16) << models::roomStatusToString(room->getStatus()) << RESET
                  << GRAY << amenitiesStr << RESET << "\n";
    }
    std::cout << std::string(90, '-') << "\n";
    std::cout << GRAY << "Total rooms listed: " << rooms.size() << RESET << "\n";
}

void TerminalUI::displayGuestsTable(const std::vector<models::Guest>& guests) {
    if (guests.empty()) {
        printWarning("No guests registered in the system yet.");
        return;
    }

    std::cout << BOLD
              << std::left
              << std::setw(14) << "Guest ID"
              << std::setw(22) << "Name"
              << std::setw(26) << "Email"
              << std::setw(16) << "Phone"
              << std::setw(10) << "Status"
              << std::setw(8)  << "Stays"
              << "Rewards" << RESET << "\n";
    std::cout << std::string(105, '-') << "\n";

    for (const auto& g : guests) {
        std::cout << std::left
                  << std::setw(14) << g.getId()
                  << std::setw(22) << g.getName()
                  << std::setw(26) << g.getEmail()
                  << std::setw(16) << g.getPhone()
                  << (g.isVIP() ? (YELLOW + BOLD + "VIP" + RESET + "       ") : "Standard  ")
                  << std::setw(8)  << g.getTotalStays()
                  << (std::to_string(g.getRewardPoints()) + " pts") << "\n";
    }
    std::cout << std::string(105, '-') << "\n";
}

void TerminalUI::displayReservationsTable(const std::vector<models::Reservation>& reservations,
                                         const services::HotelManager& manager) {
    if (reservations.empty()) {
        printWarning("No reservations recorded.");
        return;
    }

    std::cout << BOLD
              << std::left
              << std::setw(22) << "Reservation ID"
              << std::setw(18) << "Guest Name"
              << std::setw(8)  << "Room"
              << std::setw(14) << "Check-In"
              << std::setw(14) << "Check-Out"
              << std::setw(14) << "Status"
              << std::setw(12) << "Amount"
              << "Paid" << RESET << "\n";
    std::cout << std::string(105, '-') << "\n";

    for (const auto& res : reservations) {
        const auto* guest = manager.getGuest(res.getGuestId());
        std::string guestName = guest ? guest->getName() : res.getGuestId();
        if (guestName.length() > 16) guestName = guestName.substr(0, 14) + "..";

        std::string statusColor = CYAN;
        if (res.getStatus() == models::ReservationStatus::CheckedIn) statusColor = GREEN;
        else if (res.getStatus() == models::ReservationStatus::CheckedOut) statusColor = GRAY;
        else if (res.getStatus() == models::ReservationStatus::Cancelled) statusColor = RED;

        std::ostringstream amountStr;
        amountStr << std::fixed << std::setprecision(2) << "$" << res.getGrandTotal();

        std::cout << std::left
                  << std::setw(22) << res.getReservationId()
                  << std::setw(18) << guestName
                  << std::setw(8)  << res.getRoomNumber()
                  << std::setw(14) << res.getCheckInDate()
                  << std::setw(14) << res.getCheckOutDate()
                  << statusColor << std::setw(14) << models::reservationStatusToString(res.getStatus()) << RESET
                  << std::setw(12) << amountStr.str()
                  << (res.isPaid() ? (GREEN + "✓ Yes" + RESET) : (YELLOW + "✗ No" + RESET)) << "\n";
    }
    std::cout << std::string(105, '-') << "\n";
}

void TerminalUI::displayServicesCatalog(const std::vector<models::ServiceItem>& services) {
    std::cout << BOLD
              << std::left
              << std::setw(12) << "Service ID"
              << std::setw(16) << "Category"
              << std::setw(38) << "Description / Service"
              << "Price" << RESET << "\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto& s : services) {
        std::ostringstream pr;
        pr << std::fixed << std::setprecision(2) << "$" << s.getPrice();
        std::cout << std::left
                  << std::setw(12) << s.getId()
                  << std::setw(16) << s.getCategory()
                  << std::setw(38) << s.getName()
                  << GREEN << pr.str() << RESET << "\n";
    }
    std::cout << std::string(75, '-') << "\n";
}

void TerminalUI::displayAnalyticsDashboard(const services::AnalyticsReport& report) {
    std::cout << BOLD << CYAN << "┌────────────────────────────────────────────────────────┐\n";
    std::cout << "│              OPERATIONAL & FINANCIAL KPI DASHBOARD     │\n";
    std::cout << "└────────────────────────────────────────────────────────┘" << RESET << "\n\n";

    std::cout << "  " << BOLD << "Rooms Overview:" << RESET << "\n";
    std::cout << "    • Total Inventory  : " << BOLD << report.totalRooms << RESET << " rooms\n";
    std::cout << "    • Available Rooms  : " << GREEN << BOLD << report.availableRooms << RESET << "\n";
    std::cout << "    • Occupied Rooms   : " << RED << BOLD << report.occupiedRooms << RESET << "\n";
    std::cout << "    • Reserved Rooms   : " << YELLOW << BOLD << report.reservedRooms << RESET << "\n";
    std::cout << "    • Under Maintenance: " << MAGENTA << report.maintenanceRooms << RESET << "\n";

    std::cout << "\n  " << BOLD << "Performance Metrics:" << RESET << "\n";
    std::cout << "    • Current Occupancy Rate: " << (report.occupancyRate >= 50.0 ? GREEN : YELLOW)
              << std::fixed << std::setprecision(1) << report.occupancyRate << "%" << RESET << "\n";
    std::cout << "    • Active In-House Guests: " << report.activeGuests << " guests\n";
    std::cout << "    • Total Bookings Handled: " << report.totalReservations << " reservations\n";
    std::cout << "    • Total Realized Revenue: " << GREEN << BOLD
              << "$" << std::fixed << std::setprecision(2) << report.totalRevenue << RESET << "\n";
}

std::string TerminalUI::readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    // Trim leading/trailing whitespace
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = line.find_last_not_of(" \t\r\n");
    return line.substr(first, (last - first + 1));
}

int TerminalUI::readInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::cout << prompt;
        std::string input;
        std::getline(std::cin, input);
        try {
            int val = std::stoi(input);
            if (val >= minVal && val <= maxVal) {
                return val;
            }
            std::cout << RED << "Input out of range (" << minVal << " to " << maxVal << "). Please re-enter.\n" << RESET;
        } catch (...) {
            std::cout << RED << "Invalid integer. Please enter a valid number.\n" << RESET;
        }
    }
}

double TerminalUI::readDouble(const std::string& prompt, double minVal, double maxVal) {
    while (true) {
        std::cout << prompt;
        std::string input;
        std::getline(std::cin, input);
        try {
            double val = std::stod(input);
            if (val >= minVal && val <= maxVal) {
                return val;
            }
            std::cout << RED << "Input out of range (" << minVal << " to " << maxVal << "). Please re-enter.\n" << RESET;
        } catch (...) {
            std::cout << RED << "Invalid decimal number. Please re-enter.\n" << RESET;
        }
    }
}

std::string TerminalUI::readDate(const std::string& prompt, const std::string& defaultVal) {
    while (true) {
        std::string promptWithDefault = prompt;
        if (!defaultVal.empty()) {
            promptWithDefault += " [" + defaultVal + "]: ";
        } else {
            promptWithDefault += " (YYYY-MM-DD): ";
        }

        std::string dateStr = readLine(promptWithDefault);
        if (dateStr.empty() && !defaultVal.empty()) {
            return defaultVal;
        }

        if (utils::DateTimeUtils::isValidDateFormat(dateStr)) {
            return dateStr;
        }
        std::cout << RED << "Invalid date format. Expected YYYY-MM-DD (e.g. 2026-11-15).\n" << RESET;
    }
}

bool TerminalUI::readConfirmation(const std::string& prompt, bool defaultYes) {
    std::string p = prompt + (defaultYes ? " [Y/n]: " : " [y/N]: ");
    std::string resp = readLine(p);
    if (resp.empty()) return defaultYes;
    char c = static_cast<char>(std::tolower(resp[0]));
    return (c == 'y');
}

} // namespace hotel::ui
