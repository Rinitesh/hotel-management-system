#include "Models/Room.hpp"
#include <algorithm>

namespace hotel::models {

std::string roomTypeToString(RoomType type) {
    switch (type) {
        case RoomType::Standard: return "Standard";
        case RoomType::Deluxe: return "Deluxe";
        case RoomType::Suite: return "Suite";
        case RoomType::Presidential: return "Presidential";
    }
    return "Unknown";
}

RoomType stringToRoomType(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    if (s == "deluxe") return RoomType::Deluxe;
    if (s == "suite") return RoomType::Suite;
    if (s == "presidential") return RoomType::Presidential;
    return RoomType::Standard;
}

std::string roomStatusToString(RoomStatus status) {
    switch (status) {
        case RoomStatus::Available: return "Available";
        case RoomStatus::Occupied: return "Occupied";
        case RoomStatus::Reserved: return "Reserved";
        case RoomStatus::Maintenance: return "Maintenance";
    }
    return "Unknown";
}

RoomStatus stringToRoomStatus(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    if (s == "occupied") return RoomStatus::Occupied;
    if (s == "reserved") return RoomStatus::Reserved;
    if (s == "maintenance") return RoomStatus::Maintenance;
    return RoomStatus::Available;
}

Room::Room(int roomNumber, RoomType type, double baseRate, int capacity,
           RoomStatus status, std::vector<std::string> amenities)
    : roomNumber_(roomNumber),
      type_(type),
      baseRate_(baseRate),
      capacity_(capacity),
      status_(status),
      amenities_(std::move(amenities)) {}

double Room::calculateCost(int nights, const patterns::PricingStrategy& strategy) const {
    return strategy.calculateTotal(baseRate_, nights);
}

} // namespace hotel::models
