#include "Services/HotelManager.hpp"
#include "Utils/DateTimeUtils.hpp"
#include "Utils/Exceptions.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace hotel::services {

HotelManager::HotelManager(std::string dataDir)
    : dataDir_(std::move(dataDir)),
      storage_(dataDir_) {
    setupAvailableServices();
    initialize();
}

void HotelManager::setupAvailableServices() {
    availableServices_ = {
        models::ServiceItem("SRV-101", "Continental Breakfast Buffet", 25.0, "Dining"),
        models::ServiceItem("SRV-102", "Chef's Three-Course Dinner", 65.0, "Dining"),
        models::ServiceItem("SRV-103", "Full Day Spa & Sauna Pass", 95.0, "Wellness"),
        models::ServiceItem("SRV-104", "Express Laundry & Dry Clean", 30.0, "Housekeeping"),
        models::ServiceItem("SRV-105", "VIP Airport Chauffeur Transfer", 55.0, "Transport"),
        models::ServiceItem("SRV-106", "Premium Wine & Artisan Cheese Board", 45.0, "Dining"),
        models::ServiceItem("SRV-107", "Late Check-Out Privilege (2 PM)", 40.0, "FrontDesk")
    };
}

void HotelManager::initialize() {
    rooms_ = storage_.loadRooms();
    guests_ = storage_.loadGuests();
    reservations_ = storage_.loadReservations();
}

void HotelManager::persist() {
    storage_.saveRooms(rooms_);
    storage_.saveGuests(guests_);
    storage_.saveReservations(reservations_);
}

void HotelManager::addRoom(std::unique_ptr<models::Room> room) {
    if (!room) return;
    for (const auto& r : rooms_) {
        if (r->getRoomNumber() == room->getRoomNumber()) {
            throw InvalidOperationException("Room " + std::to_string(room->getRoomNumber()) + " already exists.");
        }
    }
    rooms_.push_back(std::move(room));
    std::sort(rooms_.begin(), rooms_.end(), [](const auto& a, const auto& b) {
        return a->getRoomNumber() < b->getRoomNumber();
    });
    persist();
}

models::Room* HotelManager::getRoom(int roomNumber) {
    for (auto& r : rooms_) {
        if (r->getRoomNumber() == roomNumber) {
            return r.get();
        }
    }
    return nullptr;
}

const models::Room* HotelManager::getRoom(int roomNumber) const {
    for (const auto& r : rooms_) {
        if (r->getRoomNumber() == roomNumber) {
            return r.get();
        }
    }
    return nullptr;
}

std::vector<const models::Room*> HotelManager::getAvailableRooms(std::optional<models::RoomType> filterType) const {
    std::vector<const models::Room*> result;
    for (const auto& r : rooms_) {
        if (r->getStatus() == models::RoomStatus::Available) {
            if (!filterType.has_value() || r->getType() == filterType.value()) {
                result.push_back(r.get());
            }
        }
    }
    return result;
}

void HotelManager::updateRoomStatus(int roomNumber, models::RoomStatus status) {
    auto* room = getRoom(roomNumber);
    if (!room) {
        throw RoomNotFoundException(roomNumber);
    }
    room->setStatus(status);
    persist();
}

models::Guest& HotelManager::registerGuest(std::string name, std::string email, std::string phone, std::string idProof) {
    if (name.empty()) {
        throw InvalidOperationException("Guest name cannot be empty.");
    }
    std::string guestId = utils::DateTimeUtils::generateId("GST");
    models::Guest guest(guestId, std::move(name), std::move(email), std::move(phone), std::move(idProof), false, 0, 0);
    guests_[guestId] = guest;
    persist();
    return guests_[guestId];
}

models::Guest* HotelManager::getGuest(const std::string& guestId) {
    auto it = guests_.find(guestId);
    if (it != guests_.end()) {
        return &(it->second);
    }
    return nullptr;
}

const models::Guest* HotelManager::getGuest(const std::string& guestId) const {
    auto it = guests_.find(guestId);
    if (it != guests_.end()) {
        return &(it->second);
    }
    return nullptr;
}

std::vector<models::Guest> HotelManager::getAllGuests() const {
    std::vector<models::Guest> list;
    list.reserve(guests_.size());
    for (const auto& [_, g] : guests_) {
        list.push_back(g);
    }
    return list;
}

models::Guest* HotelManager::findGuestByQuery(const std::string& query) {
    std::string q = query;
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);

    for (auto& [_, g] : guests_) {
        std::string name = g.getName();
        std::string email = g.getEmail();
        std::string phone = g.getPhone();
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(email.begin(), email.end(), email.begin(), ::tolower);

        if (g.getId() == query || name.find(q) != std::string::npos ||
            email.find(q) != std::string::npos || phone.find(q) != std::string::npos) {
            return &g;
        }
    }
    return nullptr;
}

std::string HotelManager::createReservation(const std::string& guestId, int roomNumber,
                                          const std::string& checkInDate, const std::string& checkOutDate,
                                          const patterns::PricingStrategy* customStrategy) {
    auto* guest = getGuest(guestId);
    if (!guest) {
        throw GuestNotFoundException(guestId);
    }

    auto* room = getRoom(roomNumber);
    if (!room) {
        throw RoomNotFoundException(roomNumber);
    }

    if (room->getStatus() != models::RoomStatus::Available) {
        throw RoomNotAvailableException(roomNumber, models::roomStatusToString(room->getStatus()));
    }

    int nights = utils::DateTimeUtils::calculateDaysBetween(checkInDate, checkOutDate);
    if (nights <= 0) {
        throw InvalidOperationException("Check-out date must be after check-in date (minimum 1 night stay).");
    }

    // Determine pricing strategy
    std::unique_ptr<patterns::PricingStrategy> autoStrategy;
    const patterns::PricingStrategy* activeStrategy = customStrategy;

    if (!activeStrategy) {
        if (guest->isVIP()) {
            autoStrategy = std::make_unique<patterns::VIPPricingStrategy>(15.0);
        } else if (nights >= 7) {
            autoStrategy = std::make_unique<patterns::ExtendedStayPricingStrategy>(7, 10.0);
        } else {
            autoStrategy = std::make_unique<patterns::StandardPricingStrategy>();
        }
        activeStrategy = autoStrategy.get();
    }

    double discountPct = guest->isVIP() ? 15.0 : (nights >= 7 ? 10.0 : 0.0);

    std::string resId = utils::DateTimeUtils::generateId("RES");
    models::Reservation res(resId, guestId, roomNumber, checkInDate, checkOutDate,
                            nights, room->getBaseRate(), models::ReservationStatus::Confirmed,
                            utils::DateTimeUtils::getCurrentTimestamp());

    res.recalculateTotals(discountPct, 0.12);

    room->setStatus(models::RoomStatus::Reserved);
    reservations_[resId] = res;
    persist();
    return resId;
}

void HotelManager::checkInGuest(const std::string& reservationId) {
    auto* res = getReservation(reservationId);
    if (!res) {
        throw ReservationNotFoundException(reservationId);
    }

    auto* room = getRoom(res->getRoomNumber());
    if (!room) {
        throw RoomNotFoundException(res->getRoomNumber());
    }

    res->checkIn();
    room->setStatus(models::RoomStatus::Occupied);
    persist();
}

void HotelManager::checkOutGuest(const std::string& reservationId, const std::string& paymentMethod) {
    auto* res = getReservation(reservationId);
    if (!res) {
        throw ReservationNotFoundException(reservationId);
    }

    auto* room = getRoom(res->getRoomNumber());
    if (room) {
        room->setStatus(models::RoomStatus::Available);
    }

    res->checkOut(paymentMethod);

    // Update guest loyalty stats
    auto* guest = getGuest(res->getGuestId());
    if (guest) {
        guest->addStay(res->getNumberOfNights());
    }

    persist();
}

void HotelManager::cancelReservation(const std::string& reservationId) {
    auto* res = getReservation(reservationId);
    if (!res) {
        throw ReservationNotFoundException(reservationId);
    }

    auto* room = getRoom(res->getRoomNumber());
    if (room && (room->getStatus() == models::RoomStatus::Reserved || room->getStatus() == models::RoomStatus::Occupied)) {
        room->setStatus(models::RoomStatus::Available);
    }

    res->cancel();
    persist();
}

models::Reservation* HotelManager::getReservation(const std::string& reservationId) {
    auto it = reservations_.find(reservationId);
    if (it != reservations_.end()) {
        return &(it->second);
    }
    return nullptr;
}

const models::Reservation* HotelManager::getReservation(const std::string& reservationId) const {
    auto it = reservations_.find(reservationId);
    if (it != reservations_.end()) {
        return &(it->second);
    }
    return nullptr;
}

std::vector<models::Reservation> HotelManager::getAllReservations() const {
    std::vector<models::Reservation> list;
    list.reserve(reservations_.size());
    for (const auto& [_, r] : reservations_) {
        list.push_back(r);
    }
    return list;
}

std::vector<models::Reservation> HotelManager::getActiveReservations() const {
    std::vector<models::Reservation> list;
    for (const auto& [_, r] : reservations_) {
        if (r.getStatus() == models::ReservationStatus::Confirmed ||
            r.getStatus() == models::ReservationStatus::CheckedIn) {
            list.push_back(r);
        }
    }
    return list;
}

void HotelManager::orderService(const std::string& reservationId, const std::string& serviceId) {
    auto* res = getReservation(reservationId);
    if (!res) {
        throw ReservationNotFoundException(reservationId);
    }

    auto srvIt = std::find_if(availableServices_.begin(), availableServices_.end(),
                              [&serviceId](const models::ServiceItem& s) {
                                  return s.getId() == serviceId;
                              });

    if (srvIt == availableServices_.end()) {
        throw InvalidOperationException("Service ID '" + serviceId + "' is not available in our catalog.");
    }

    models::ServiceItem item = *srvIt;
    item.setTimestamp(utils::DateTimeUtils::getCurrentTimestamp());

    auto* guest = getGuest(res->getGuestId());
    double discountPct = (guest && guest->isVIP()) ? 15.0 : 0.0;

    res->addService(item);
    res->recalculateTotals(discountPct, 0.12);
    persist();
}

AnalyticsReport HotelManager::getAnalytics() const {
    AnalyticsReport report{};
    report.totalRooms = static_cast<int>(rooms_.size());
    report.totalReservations = static_cast<int>(reservations_.size());

    for (const auto& r : rooms_) {
        switch (r->getStatus()) {
            case models::RoomStatus::Occupied: report.occupiedRooms++; break;
            case models::RoomStatus::Reserved: report.reservedRooms++; break;
            case models::RoomStatus::Available: report.availableRooms++; break;
            case models::RoomStatus::Maintenance: report.maintenanceRooms++; break;
        }
    }

    if (report.totalRooms > 0) {
        report.occupancyRate = (static_cast<double>(report.occupiedRooms) / report.totalRooms) * 100.0;
    }

    for (const auto& [_, res] : reservations_) {
        if (res.getStatus() == models::ReservationStatus::CheckedOut ||
            res.getStatus() == models::ReservationStatus::CheckedIn) {
            report.totalRevenue += res.getGrandTotal();
        }
        if (res.getStatus() == models::ReservationStatus::CheckedIn) {
            report.activeGuests++;
        }
    }

    return report;
}

std::string HotelManager::generateInvoice(const std::string& reservationId) const {
    const auto* res = getReservation(reservationId);
    if (!res) {
        throw ReservationNotFoundException(reservationId);
    }

    const auto* guest = getGuest(res->getGuestId());
    const auto* room = getRoom(res->getRoomNumber());

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "========================================================================\n";
    ss << "                      GRAND HORIZON HOTEL & SUITES                      \n";
    ss << "                       OFFICIAL FOLIO & INVOICE                         \n";
    ss << "========================================================================\n";
    ss << " Reservation ID : " << res->getReservationId() << "\n";
    ss << " Booking Date   : " << res->getCreatedAt() << "\n";
    ss << " Invoice Date   : " << utils::DateTimeUtils::getCurrentTimestamp() << "\n";
    ss << " Status         : " << models::reservationStatusToString(res->getStatus()) << "\n";
    ss << " Payment Status : " << (res->isPaid() ? ("PAID via " + res->getPaymentMethod()) : "PENDING") << "\n";
    ss << "------------------------------------------------------------------------\n";
    ss << " GUEST DETAILS:\n";
    if (guest) {
        ss << "   Name         : " << guest->getName() << (guest->isVIP() ? " [VIP MEMBER]" : "") << "\n";
        ss << "   Email        : " << guest->getEmail() << "\n";
        ss << "   Phone        : " << guest->getPhone() << "\n";
        ss << "   ID / Passport: " << guest->getIdProof() << "\n";
    } else {
        ss << "   Guest ID     : " << res->getGuestId() << " (Unregistered)\n";
    }
    ss << "------------------------------------------------------------------------\n";
    ss << " ROOM ACCOMMODATION:\n";
    ss << "   Room Number  : Room " << res->getRoomNumber()
       << (room ? (" (" + room->getTypeName() + ")") : "") << "\n";
    ss << "   Stay Period  : " << res->getCheckInDate() << " to " << res->getCheckOutDate()
       << " (" << res->getNumberOfNights() << " night" << (res->getNumberOfNights() > 1 ? "s" : "") << ")\n";
    ss << "   Nightly Rate : $" << res->getRoomRatePerNight() << "\n";
    ss << "   Room Subtotal: $" << res->getRoomTotal() << "\n";

    if (!res->getServices().empty()) {
        ss << "------------------------------------------------------------------------\n";
        ss << " ANCILLARY SERVICES & ROOM CHARGES:\n";
        int idx = 1;
        for (const auto& s : res->getServices()) {
            ss << "   " << idx++ << ". [" << s.getCategory() << "] "
               << std::left << std::setw(32) << s.getName()
               << " $" << std::right << std::setw(8) << s.getPrice() << "\n";
        }
        ss << "   Services Subtotal: $" << res->getServicesTotal() << "\n";
    }

    ss << "------------------------------------------------------------------------\n";
    ss << " FINANCIAL SUMMARY:\n";
    double rawSubtotal = res->getRoomTotal() + res->getServicesTotal();
    ss << "   Gross Total       : $" << std::setw(10) << rawSubtotal << "\n";
    if (res->getDiscountAmount() > 0.0) {
        ss << "   Discount Applied  : -$" << std::setw(9) << res->getDiscountAmount() << "\n";
    }
    ss << "   Tax & Surchg (12%): $" << std::setw(10) << res->getTaxAmount() << "\n";
    ss << "   -----------------------------------------\n";
    ss << "   NET AMOUNT DUE    : $" << std::setw(10) << res->getGrandTotal() << "\n";
    ss << "========================================================================\n";
    ss << "               Thank you for staying at Grand Horizon!                  \n";
    ss << "========================================================================\n";

    return ss.str();
}

} // namespace hotel::services
