#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Patterns/PricingStrategy.hpp"

namespace hotel::models {

enum class RoomType {
    Standard,
    Deluxe,
    Suite,
    Presidential
};

enum class RoomStatus {
    Available,
    Occupied,
    Reserved,
    Maintenance
};

std::string roomTypeToString(RoomType type);
RoomType stringToRoomType(const std::string& str);

std::string roomStatusToString(RoomStatus status);
RoomStatus stringToRoomStatus(const std::string& str);

class Room {
public:
    Room(int roomNumber, RoomType type, double baseRate, int capacity,
         RoomStatus status = RoomStatus::Available,
         std::vector<std::string> amenities = {});

    virtual ~Room() = default;

    // Pure virtual methods to be implemented by concrete room types
    [[nodiscard]] virtual std::string getTypeName() const = 0;
    [[nodiscard]] virtual std::string getDescription() const = 0;
    [[nodiscard]] virtual double getServiceMultiplier() const { return 1.0; }

    // Dynamic pricing calculation using Strategy pattern
    [[nodiscard]] virtual double calculateCost(int nights, const patterns::PricingStrategy& strategy) const;

    // Getters
    [[nodiscard]] int getRoomNumber() const { return roomNumber_; }
    [[nodiscard]] RoomType getType() const { return type_; }
    [[nodiscard]] double getBaseRate() const { return baseRate_; }
    [[nodiscard]] double getNightlyRateINR() const { return baseRate_; }
    [[nodiscard]] int getCapacity() const { return capacity_; }
    [[nodiscard]] RoomStatus getStatus() const { return status_; }
    [[nodiscard]] const std::vector<std::string>& getAmenities() const { return amenities_; }

    // Setters
    void setStatus(RoomStatus newStatus) { status_ = newStatus; }
    void setBaseRate(double rate) { baseRate_ = rate; }
    void setNightlyRateINR(double rate) { baseRate_ = rate; }
    void addAmenity(const std::string& amenity) { amenities_.push_back(amenity); }

protected:
    int roomNumber_;
    RoomType type_;
    double baseRate_;
    int capacity_;
    RoomStatus status_;
    std::vector<std::string> amenities_;
};

} // namespace hotel::models
