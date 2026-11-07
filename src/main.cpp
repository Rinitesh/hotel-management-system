#include <iostream>
#include <memory>
#include <iomanip>
#include "Services/HotelManager.hpp"
#include "Patterns/RoomFactory.hpp"
#include "Patterns/PricingStrategy.hpp"
#include "Utils/TerminalUI.hpp"
#include "Utils/DateTimeUtils.hpp"
#include "Utils/Exceptions.hpp"

using namespace hotel;
using namespace hotel::ui;
using namespace hotel::models;
using namespace hotel::patterns;
using namespace hotel::services;

void handleViewAllRooms(HotelManager& manager) {
    TerminalUI::printHeader("Room Directory & Status");
    std::cout << "Filter: [1] All Rooms  [2] Only Available  [3] Only Occupied  [4] Filter by Type\n";
    int choice = TerminalUI::readInt("Select filter (1-4): ", 1, 4);

    std::vector<const Room*> list;
    if (choice == 1) {
        for (const auto& r : manager.getAllRooms()) list.push_back(r.get());
    } else if (choice == 2) {
        list = manager.getAvailableRooms();
    } else if (choice == 3) {
        for (const auto& r : manager.getAllRooms()) {
            if (r->getStatus() == RoomStatus::Occupied) list.push_back(r.get());
        }
    } else if (choice == 4) {
        std::cout << "[1] Standard  [2] Deluxe  [3] Suite  [4] Presidential\n";
        int t = TerminalUI::readInt("Select Type (1-4): ", 1, 4);
        RoomType rt = (t == 1 ? RoomType::Standard : (t == 2 ? RoomType::Deluxe : (t == 3 ? RoomType::Suite : RoomType::Presidential)));
        list = manager.getAvailableRooms(rt);
    }

    TerminalUI::displayRoomsTable(list);
    TerminalUI::pause();
}

void handleRegisterGuest(HotelManager& manager) {
    TerminalUI::printHeader("Guest Registration");
    std::string name = TerminalUI::readLine("Full Name: ");
    if (name.empty()) {
        TerminalUI::printError("Name cannot be empty.");
        TerminalUI::pause();
        return;
    }
    std::string email = TerminalUI::readLine("Email Address: ");
    std::string phone = TerminalUI::readLine("Phone Number: ");
    std::string idProof = TerminalUI::readLine("Govt ID / Passport: ");

    auto& guest = manager.registerGuest(name, email, phone, idProof);
    TerminalUI::printSuccess("Guest successfully registered!");
    std::cout << "Assigned Guest ID: " << TerminalUI::BOLD << guest.getId() << TerminalUI::RESET << "\n";
    TerminalUI::pause();
}

void handleViewGuests(HotelManager& manager) {
    TerminalUI::printHeader("Guest Directory");
    TerminalUI::displayGuestsTable(manager.getAllGuests());
    TerminalUI::pause();
}

void handleBookRoom(HotelManager& manager) {
    TerminalUI::printHeader("New Reservation / Room Booking");

    std::string guestSearch = TerminalUI::readLine("Enter Guest ID, Name, or Phone (or leave blank to register new): ");
    Guest* guest = nullptr;

    if (!guestSearch.empty()) {
        guest = manager.findGuestByQuery(guestSearch);
    }

    if (!guest) {
        TerminalUI::printInfo("Guest not found. Let's register the guest first.");
        std::string name = TerminalUI::readLine("Full Name: ");
        if (name.empty()) {
            TerminalUI::printError("Guest name cannot be empty. Booking aborted.");
            TerminalUI::pause();
            return;
        }
        std::string email = TerminalUI::readLine("Email: ");
        std::string phone = TerminalUI::readLine("Phone: ");
        std::string idProof = TerminalUI::readLine("ID Proof: ");
        guest = &manager.registerGuest(name, email, phone, idProof);
        TerminalUI::printSuccess("Guest registered with ID: " + guest->getId());
    } else {
        TerminalUI::printSuccess("Found Guest: " + guest->getName() + (guest->isVIP() ? " [VIP]" : ""));
    }

    // Show available rooms
    auto available = manager.getAvailableRooms();
    if (available.empty()) {
        TerminalUI::printError("No rooms are currently available for booking.");
        TerminalUI::pause();
        return;
    }

    std::cout << "\nAvailable Rooms:\n";
    TerminalUI::displayRoomsTable(available);

    int roomNumber = TerminalUI::readInt("Enter Room Number to book: ", 1, 9999);
    Room* selectedRoom = manager.getRoom(roomNumber);
    if (!selectedRoom || selectedRoom->getStatus() != RoomStatus::Available) {
        TerminalUI::printError("Selected room is not available.");
        TerminalUI::pause();
        return;
    }

    std::string today = utils::DateTimeUtils::getCurrentDate();
    std::string defaultCheckOut = utils::DateTimeUtils::addDaysToDate(today, 2);

    std::string checkIn = TerminalUI::readDate("Check-In Date", today);
    std::string checkOut = TerminalUI::readDate("Check-Out Date", defaultCheckOut);

    int nights = utils::DateTimeUtils::calculateDaysBetween(checkIn, checkOut);
    if (nights <= 0) {
        TerminalUI::printError("Check-out must be after check-in date.");
        TerminalUI::pause();
        return;
    }

    // Pricing rate selection
    std::cout << "\nSelect Rate Plan:\n";
    std::cout << "  [1] Standard Rate (INR " << static_cast<long long>(selectedRoom->getBaseRate()) << "/night)\n";
    std::cout << "  [2] Peak Season (+25% Holiday Surcharge)\n";
    std::cout << "  [3] Extended Stay Discount (for 7+ nights, 10% off)\n";
    if (guest->isVIP()) {
        std::cout << "  " << TerminalUI::YELLOW << "★ VIP Member 15% discount automatically applied." << TerminalUI::RESET << "\n";
    }
    int rateChoice = TerminalUI::readInt("Choose Rate Plan (1-3): ", 1, 3);

    std::unique_ptr<PricingStrategy> strategy;
    if (rateChoice == 2) {
        strategy = std::make_unique<PeakSeasonPricingStrategy>(25.0);
    } else if (rateChoice == 3) {
        strategy = std::make_unique<ExtendedStayPricingStrategy>(7, 10.0);
    }

    try {
        std::string resId = manager.createReservation(guest->getId(), roomNumber, checkIn, checkOut, strategy.get());
        TerminalUI::printSuccess("Reservation confirmed successfully!");
        std::cout << "Reservation Code: " << TerminalUI::BOLD << resId << TerminalUI::RESET << "\n";
        std::cout << "Nights: " << nights << " | Room: " << roomNumber << " (" << selectedRoom->getTypeName() << ")\n";

        if (TerminalUI::readConfirmation("Would you like to check-in this guest right now?", false)) {
            manager.checkInGuest(resId);
            TerminalUI::printSuccess("Guest is now Checked-In to Room " + std::to_string(roomNumber));
        }
    } catch (const std::exception& ex) {
        TerminalUI::printError(ex.what());
    }
    TerminalUI::pause();
}

void handleCheckIn(HotelManager& manager) {
    TerminalUI::printHeader("Guest Check-In");
    auto active = manager.getAllReservations();
    std::vector<Reservation> confirmed;
    for (const auto& r : active) {
        if (r.getStatus() == ReservationStatus::Confirmed) confirmed.push_back(r);
    }

    if (confirmed.empty()) {
        TerminalUI::printWarning("No confirmed reservations pending check-in.");
        TerminalUI::pause();
        return;
    }

    TerminalUI::displayReservationsTable(confirmed, manager);
    std::string resId = TerminalUI::readLine("Enter Reservation ID to check in: ");

    try {
        manager.checkInGuest(resId);
        TerminalUI::printSuccess("Check-in successful! Room keys issued.");
    } catch (const std::exception& ex) {
        TerminalUI::printError(ex.what());
    }
    TerminalUI::pause();
}

void handleOrderService(HotelManager& manager) {
    TerminalUI::printHeader("Order Room Service & Ancillary Amenities");

    auto active = manager.getAllReservations();
    std::vector<Reservation> checkedIn;
    for (const auto& r : active) {
        if (r.getStatus() == ReservationStatus::CheckedIn) checkedIn.push_back(r);
    }

    if (checkedIn.empty()) {
        TerminalUI::printWarning("No guests currently checked-in to order room services.");
        TerminalUI::pause();
        return;
    }

    TerminalUI::displayReservationsTable(checkedIn, manager);
    std::string resId = TerminalUI::readLine("Enter Reservation ID: ");

    auto* res = manager.getReservation(resId);
    if (!res || res->getStatus() != ReservationStatus::CheckedIn) {
        TerminalUI::printError("Invalid or inactive reservation.");
        TerminalUI::pause();
        return;
    }

    std::cout << "\nServices & Dining Menu:\n";
    TerminalUI::displayServicesCatalog(manager.getAvailableServices());

    std::string serviceId = TerminalUI::readLine("Enter Service ID (e.g., SRV-101): ");
    try {
        manager.orderService(resId, serviceId);
        TerminalUI::printSuccess("Service charged to room folio successfully!");
    } catch (const std::exception& ex) {
        TerminalUI::printError(ex.what());
    }
    TerminalUI::pause();
}

void handleCheckOut(HotelManager& manager) {
    TerminalUI::printHeader("Guest Check-Out & Final Settlement");

    auto active = manager.getAllReservations();
    std::vector<Reservation> checkedIn;
    for (const auto& r : active) {
        if (r.getStatus() == ReservationStatus::CheckedIn) checkedIn.push_back(r);
    }

    if (checkedIn.empty()) {
        TerminalUI::printWarning("No in-house guests currently checked-in.");
        TerminalUI::pause();
        return;
    }

    TerminalUI::displayReservationsTable(checkedIn, manager);
    std::string resId = TerminalUI::readLine("Enter Reservation ID to Check-Out: ");

    auto* res = manager.getReservation(resId);
    if (!res) {
        TerminalUI::printError("Reservation not found.");
        TerminalUI::pause();
        return;
    }

    // Print invoice before settlement
    std::cout << "\n" << manager.generateInvoice(resId) << "\n";

    std::cout << "Select Payment Method:\n";
    std::cout << "  [1] Credit Card  [2] Debit Card  [3] UPI / Digital Wallet  [4] Cash  [5] Corporate Bill\n";
    int pChoice = TerminalUI::readInt("Payment Method (1-5): ", 1, 5);

    std::string paymentMethod = "Credit Card";
    if (pChoice == 2) paymentMethod = "Debit Card";
    else if (pChoice == 3) paymentMethod = "UPI";
    else if (pChoice == 4) paymentMethod = "Cash";
    else if (pChoice == 5) paymentMethod = "Corporate Bill";

    try {
        manager.checkOutGuest(resId, paymentMethod);
        TerminalUI::printSuccess("Checkout completed! Payment of INR " +
                                std::to_string(static_cast<long long>(res->getGrandTotal())) + " recorded via " + paymentMethod);
        std::cout << "Room " << res->getRoomNumber() << " is now marked Available for cleaning/re-booking.\n";
    } catch (const std::exception& ex) {
        TerminalUI::printError(ex.what());
    }
    TerminalUI::pause();
}

void handleViewFolio(HotelManager& manager) {
    TerminalUI::printHeader("View Folio / Print Invoice");
    std::string resId = TerminalUI::readLine("Enter Reservation ID: ");
    try {
        std::string invoice = manager.generateInvoice(resId);
        std::cout << "\n" << invoice << "\n";
    } catch (const std::exception& ex) {
        TerminalUI::printError(ex.what());
    }
    TerminalUI::pause();
}

void handleCancelReservation(HotelManager& manager) {
    TerminalUI::printHeader("Cancel Reservation");
    std::string resId = TerminalUI::readLine("Enter Reservation ID to cancel: ");
    try {
        if (TerminalUI::readConfirmation("Are you sure you want to cancel reservation " + resId + "?", false)) {
            manager.cancelReservation(resId);
            TerminalUI::printSuccess("Reservation successfully cancelled. Room released.");
        }
    } catch (const std::exception& ex) {
        TerminalUI::printError(ex.what());
    }
    TerminalUI::pause();
}

void handleAddRoom(HotelManager& manager) {
    TerminalUI::printHeader("Add New Room to Hotel Inventory");
    int roomNum = TerminalUI::readInt("Enter New Room Number (e.g. 501): ", 100, 9999);

    if (manager.getRoom(roomNum) != nullptr) {
        TerminalUI::printError("Room " + std::to_string(roomNum) + " already exists!");
        TerminalUI::pause();
        return;
    }

    std::cout << "Room Category:\n";
    std::cout << "  [1] Standard Room (INR 1800/night default)\n";
    std::cout << "  [2] Deluxe Room (INR 2600/night default)\n";
    std::cout << "  [3] Executive Suite (INR 4200/night default)\n";
    std::cout << "  [4] Presidential Suite (INR 8500/night default)\n";
    int typeChoice = TerminalUI::readInt("Select Category (1-4): ", 1, 4);

    RoomType rt = (typeChoice == 1 ? RoomType::Standard : (typeChoice == 2 ? RoomType::Deluxe : (typeChoice == 3 ? RoomType::Suite : RoomType::Presidential)));

    double customRate = TerminalUI::readDouble("Nightly Rate INR (0 for default): ", 0.0, 100000.0);
    auto newRoom = RoomFactory::createRoom(roomNum, rt, (customRate > 0 ? customRate : -1.0));

    manager.addRoom(std::move(newRoom));
    TerminalUI::printSuccess("Room " + std::to_string(roomNum) + " added successfully!");
    TerminalUI::pause();
}

void handleAnalytics(HotelManager& manager) {
    TerminalUI::printHeader("Hotel Operations & Financial Analytics");
    auto report = manager.getAnalytics();
    TerminalUI::displayAnalyticsDashboard(report);
    TerminalUI::pause();
}

int main() {
    HotelManager manager("data");

    bool running = true;
    while (running) {
        TerminalUI::printBanner();
        std::cout << TerminalUI::BOLD << "MAIN CONTROL PANEL" << TerminalUI::RESET << "\n";
        std::cout << "  " << TerminalUI::CYAN << "[1]" << TerminalUI::RESET << "  View All Rooms & Live Inventory\n";
        std::cout << "  " << TerminalUI::CYAN << "[2]" << TerminalUI::RESET << "  Book a Room (New Reservation)\n";
        std::cout << "  " << TerminalUI::CYAN << "[3]" << TerminalUI::RESET << "  Guest Check-In\n";
        std::cout << "  " << TerminalUI::CYAN << "[4]" << TerminalUI::RESET << "  Order Room Service & Amenities\n";
        std::cout << "  " << TerminalUI::CYAN << "[5]" << TerminalUI::RESET << "  Guest Check-Out & Billing Folio\n";
        std::cout << "  " << TerminalUI::CYAN << "[6]" << TerminalUI::RESET << "  View / Print Folio Invoice\n";
        std::cout << "  " << TerminalUI::CYAN << "[7]" << TerminalUI::RESET << "  Cancel Reservation\n";
        std::cout << "  " << TerminalUI::CYAN << "[8]" << TerminalUI::RESET << "  Guest Directory & Profiles\n";
        std::cout << "  " << TerminalUI::CYAN << "[9]" << TerminalUI::RESET << "  Register New Guest\n";
        std::cout << "  " << TerminalUI::CYAN << "[10]" << TerminalUI::RESET << " Add New Room (Admin)\n";
        std::cout << "  " << TerminalUI::CYAN << "[11]" << TerminalUI::RESET << " Executive KPI & Financial Analytics\n";
        std::cout << "  " << TerminalUI::CYAN << "[0]" << TerminalUI::RESET << "  Save & Exit\n\n";

        int choice = TerminalUI::readInt("Select an option (0-11): ", 0, 11);

        switch (choice) {
            case 1: handleViewAllRooms(manager); break;
            case 2: handleBookRoom(manager); break;
            case 3: handleCheckIn(manager); break;
            case 4: handleOrderService(manager); break;
            case 5: handleCheckOut(manager); break;
            case 6: handleViewFolio(manager); break;
            case 7: handleCancelReservation(manager); break;
            case 8: handleViewGuests(manager); break;
            case 9: handleRegisterGuest(manager); break;
            case 10: handleAddRoom(manager); break;
            case 11: handleAnalytics(manager); break;
            case 0:
                manager.persist();
                TerminalUI::printSuccess("All changes saved to persistent storage. Goodbye!");
                running = false;
                break;
            default:
                break;
        }
    }

    return 0;
}
