#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include "Models/Room.hpp"
#include "Models/Guest.hpp"
#include "Models/Reservation.hpp"
#include "Models/ServiceItem.hpp"
#include "Patterns/PricingStrategy.hpp"
#include "Services/StorageManager.hpp"

namespace hotel::services {

struct AnalyticsReport {
    int totalRooms = 0;
    int occupiedRooms = 0;
    int reservedRooms = 0;
    int availableRooms = 0;
    int maintenanceRooms = 0;
    double occupancyRate = 0.0;
    double totalRevenue = 0.0;
    int totalReservations = 0;
    int activeGuests = 0;
};

class HotelManager {
public:
    explicit HotelManager(std::string dataDir = "data");

    // Initialization & Persistence
    void initialize();
    void persist();

    // Room Management
    void addRoom(std::unique_ptr<models::Room> room);
    models::Room* getRoom(int roomNumber);
    const models::Room* getRoom(int roomNumber) const;
    const std::vector<std::unique_ptr<models::Room>>& getAllRooms() const { return rooms_; }
    std::vector<const models::Room*> getAvailableRooms(std::optional<models::RoomType> filterType = std::nullopt) const;
    void updateRoomStatus(int roomNumber, models::RoomStatus status);

    // Guest Management
    models::Guest& registerGuest(std::string name, std::string email, std::string phone, std::string idProof);
    models::Guest* getGuest(const std::string& guestId);
    const models::Guest* getGuest(const std::string& guestId) const;
    std::vector<models::Guest> getAllGuests() const;
    models::Guest* findGuestByQuery(const std::string& query);

    // Reservation Management
    std::string createReservation(const std::string& guestId, int roomNumber,
                                  const std::string& checkInDate, const std::string& checkOutDate,
                                  const patterns::PricingStrategy* customStrategy = nullptr);
    void checkInGuest(const std::string& reservationId);
    void checkOutGuest(const std::string& reservationId, const std::string& paymentMethod);
    void cancelReservation(const std::string& reservationId);
    models::Reservation* getReservation(const std::string& reservationId);
    const models::Reservation* getReservation(const std::string& reservationId) const;
    std::vector<models::Reservation> getAllReservations() const;
    std::vector<models::Reservation> getActiveReservations() const;

    // Service Orders
    const std::vector<models::ServiceItem>& getAvailableServices() const { return availableServices_; }
    void orderService(const std::string& reservationId, const std::string& serviceId);

    // Analytics & Billing
    [[nodiscard]] AnalyticsReport getAnalytics() const;
    [[nodiscard]] std::string generateInvoice(const std::string& reservationId) const;

private:
    std::string dataDir_;
    StorageManager storage_;
    std::vector<std::unique_ptr<models::Room>> rooms_;
    std::unordered_map<std::string, models::Guest> guests_;
    std::unordered_map<std::string, models::Reservation> reservations_;
    std::vector<models::ServiceItem> availableServices_;

    void setupAvailableServices();
};

} // namespace hotel::services
