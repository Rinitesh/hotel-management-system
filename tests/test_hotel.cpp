#include <iostream>
#include <cassert>
#include <cmath>
#include <memory>
#include "Patterns/RoomFactory.hpp"
#include "Patterns/PricingStrategy.hpp"
#include "Models/Guest.hpp"
#include "Models/Reservation.hpp"
#include "Services/HotelManager.hpp"
#include "Utils/DateTimeUtils.hpp"
#include "Utils/Exceptions.hpp"

using namespace hotel;
using namespace hotel::models;
using namespace hotel::patterns;
using namespace hotel::services;
using namespace hotel::utils;

static int testsPassed = 0;
static int totalTests = 0;

#define ASSERT_TRUE(condition, msg) \
    do { \
        totalTests++; \
        if (!(condition)) { \
            std::cerr << "\033[31m[FAILED]\033[0m " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        } else { \
            testsPassed++; \
            std::cout << "\033[32m[PASSED]\033[0m " << msg << "\n"; \
        } \
    } while(0)

#define ASSERT_EQUAL(a, b, msg) ASSERT_TRUE((a) == (b), msg)
#define ASSERT_NEAR(a, b, epsilon, msg) ASSERT_TRUE(std::fabs((a) - (b)) < (epsilon), msg)

void testDateTimeUtils() {
    std::cout << "\n=== Running DateTimeUtils Tests ===\n";
    ASSERT_TRUE(DateTimeUtils::isValidDateFormat("2026-11-07"), "Valid date format recognized");
    ASSERT_TRUE(!DateTimeUtils::isValidDateFormat("2026-13-01"), "Invalid month recognized");
    ASSERT_TRUE(!DateTimeUtils::isValidDateFormat("invalid-date"), "Invalid string format recognized");

    int days = DateTimeUtils::calculateDaysBetween("2026-11-01", "2026-11-05");
    ASSERT_EQUAL(days, 4, "Calculate days between 2026-11-01 and 2026-11-05 is 4");

    std::string nextWeek = DateTimeUtils::addDaysToDate("2026-11-01", 7);
    ASSERT_EQUAL(nextWeek, "2026-11-08", "Adding 7 days to 2026-11-01 yields 2026-11-08");
}

void testRoomPolymorphismAndFactory() {
    std::cout << "\n=== Running Room Factory & Polymorphism Tests ===\n";
    auto standard = RoomFactory::createRoom(101, RoomType::Standard);
    ASSERT_EQUAL(standard->getRoomNumber(), 101, "Standard room has correct number");
    ASSERT_EQUAL(standard->getTypeName(), "Standard Room", "Standard room type name matches");
    ASSERT_NEAR(standard->getBaseRate(), 80.0, 0.01, "Standard room base rate default is 80.0");

    auto deluxe = RoomFactory::createRoom(201, RoomType::Deluxe);
    ASSERT_EQUAL(deluxe->getTypeName(), "Deluxe Room", "Deluxe room type name matches");
    ASSERT_NEAR(deluxe->getBaseRate(), 150.0, 0.01, "Deluxe room base rate default is 150.0");

    auto suite = RoomFactory::createRoom(301, RoomType::Suite);
    ASSERT_EQUAL(suite->getTypeName(), "Executive Suite", "Suite room type name matches");

    auto presidential = RoomFactory::createRoom(401, RoomType::Presidential);
    ASSERT_EQUAL(presidential->getTypeName(), "Presidential Suite", "Presidential suite matches");
}

void testPricingStrategies() {
    std::cout << "\n=== Running Pricing Strategy Tests ===\n";
    StandardPricingStrategy standard;
    double stdCost = standard.calculateTotal(100.0, 3);
    ASSERT_NEAR(stdCost, 300.0, 0.01, "Standard pricing: 100 * 3 = 300");

    VIPPricingStrategy vip(15.0);
    double vipCost = vip.calculateTotal(100.0, 3);
    ASSERT_NEAR(vipCost, 255.0, 0.01, "VIP 15% discount: 300 * 0.85 = 255");

    PeakSeasonPricingStrategy peak(25.0);
    double peakCost = peak.calculateTotal(100.0, 2);
    ASSERT_NEAR(peakCost, 250.0, 0.01, "Peak surcharge 25%: 200 * 1.25 = 250");

    ExtendedStayPricingStrategy ext(7, 10.0);
    double shortStay = ext.calculateTotal(100.0, 4);
    double longStay = ext.calculateTotal(100.0, 10);
    ASSERT_NEAR(shortStay, 400.0, 0.01, "Extended stay under threshold gets normal rate");
    ASSERT_NEAR(longStay, 900.0, 0.01, "Extended stay >= 7 nights gets 10% discount: 1000 * 0.9 = 900");
}

void testGuestAndLoyalty() {
    std::cout << "\n=== Running Guest & Loyalty Tests ===\n";
    Guest guest("GST-999", "John Doe", "john@example.com", "+1-555-1234", "PASS-12345");
    ASSERT_TRUE(!guest.isVIP(), "New guest is initially not VIP");

    guest.addStay(3); // 3 nights -> 300 points, 1 stay
    ASSERT_TRUE(!guest.isVIP(), "Guest not VIP after 1 stay");

    guest.addStay(4); // 4 nights -> +400 pts = 700 pts, 2 stays
    ASSERT_TRUE(!guest.isVIP(), "Guest not VIP after 2 stays");

    guest.addStay(3); // 3 nights -> +300 pts = 1000 pts, 3 stays
    ASSERT_TRUE(guest.isVIP(), "Guest automatically upgraded to VIP after 3 stays / 1000 pts");
}

void testReservationFolioAndLifecycle() {
    std::cout << "\n=== Running Reservation Lifecycle Tests ===\n";
    Reservation res("RES-TEST-01", "GST-01", 101, "2026-11-10", "2026-11-13", 3, 100.0);
    ASSERT_EQUAL(res.getNumberOfNights(), 3, "Reservation has 3 nights");
    ASSERT_NEAR(res.getRoomTotal(), 300.0, 0.01, "Room total is 300.0");
    ASSERT_NEAR(res.getGrandTotal(), 336.0, 0.01, "Grand total with 12% tax is 336.0");

    // Add service
    res.addService(ServiceItem("SRV-101", "Breakfast Buffet", 25.0, "Dining"));
    ASSERT_NEAR(res.getServicesTotal(), 25.0, 0.01, "Services total is 25.0");
    // Subtotal 325.0, Tax 12% = 39.0, Grand Total = 364.0
    ASSERT_NEAR(res.getGrandTotal(), 364.0, 0.01, "Grand total after breakfast is 364.0");

    // Check-in
    res.checkIn();
    ASSERT_TRUE(res.getStatus() == ReservationStatus::CheckedIn, "Reservation is CheckedIn");

    // Check-out
    res.checkOut("Credit Card");
    ASSERT_TRUE(res.getStatus() == ReservationStatus::CheckedOut, "Reservation is CheckedOut");
    ASSERT_TRUE(res.isPaid(), "Reservation marked Paid");
    ASSERT_EQUAL(res.getPaymentMethod(), "Credit Card", "Payment method is Credit Card");
}

void testHotelManagerIntegration() {
    std::cout << "\n=== Running HotelManager Integration Tests ===\n";
    // Use test data directory
    HotelManager mgr("data_test");

    // Add a room
    mgr.addRoom(RoomFactory::createRoom(901, RoomType::Deluxe, 200.0));
    const auto* r901 = mgr.getRoom(901);
    ASSERT_TRUE(r901 != nullptr, "Room 901 retrieved");
    ASSERT_TRUE(r901->getStatus() == RoomStatus::Available, "Room 901 initially available");

    // Register guest
    auto& guest = mgr.registerGuest("Integration Tester", "test@domain.com", "+1-000-0000", "ID-TEST");
    ASSERT_TRUE(!guest.getId().empty(), "Guest ID generated");

    // Book room
    std::string resId = mgr.createReservation(guest.getId(), 901, "2026-12-01", "2026-12-04");
    ASSERT_TRUE(!resId.empty(), "Reservation ID generated");

    // Verify room status changed to Reserved
    r901 = mgr.getRoom(901);
    ASSERT_TRUE(r901->getStatus() == RoomStatus::Reserved, "Room 901 status is now Reserved");

    // Cannot double book
    bool caughtException = false;
    try {
        mgr.createReservation(guest.getId(), 901, "2026-12-01", "2026-12-04");
    } catch (const RoomNotAvailableException&) {
        caughtException = true;
    }
    ASSERT_TRUE(caughtException, "Double booking throws RoomNotAvailableException");

    // Check in
    mgr.checkInGuest(resId);
    r901 = mgr.getRoom(901);
    ASSERT_TRUE(r901->getStatus() == RoomStatus::Occupied, "Room 901 status is Occupied after check-in");

    // Add service
    mgr.orderService(resId, "SRV-101");

    // Check out
    mgr.checkOutGuest(resId, "UPI");
    r901 = mgr.getRoom(901);
    ASSERT_TRUE(r901->getStatus() == RoomStatus::Available, "Room 901 returned to Available after checkout");

    auto* res = mgr.getReservation(resId);
    ASSERT_TRUE(res->isPaid(), "Reservation marked as paid after checkout");
}

int main() {
    std::cout << "\033[1m===========================================\n";
    std::cout << "  RUNNING HOTEL MANAGEMENT SYSTEM TEST SUITE \n";
    std::cout << "===========================================\033[0m\n";

    testDateTimeUtils();
    testRoomPolymorphismAndFactory();
    testPricingStrategies();
    testGuestAndLoyalty();
    testReservationFolioAndLifecycle();
    testHotelManagerIntegration();

    std::cout << "\n\033[1m===========================================\n";
    if (testsPassed == totalTests) {
        std::cout << "\033[32mALL " << testsPassed << "/" << totalTests << " TESTS PASSED SUCCESSFULLY! ✓\033[0m\n";
    } else {
        std::cout << "\033[31mSOME TESTS FAILED: " << testsPassed << "/" << totalTests << " passed.\033[0m\n";
    }
    std::cout << "\033[1m===========================================\033[0m\n";

    return (testsPassed == totalTests ? 0 : 1);
}
