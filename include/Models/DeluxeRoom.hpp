#pragma once

#include "Models/Room.hpp"

namespace hotel::models {

class DeluxeRoom : public Room {
public:
    explicit DeluxeRoom(int roomNumber, double baseRate = 150.0, RoomStatus status = RoomStatus::Available)
        : Room(roomNumber, RoomType::Deluxe, baseRate, 3, status,
               {"King Bed", "Private Balcony", "City/Garden View", "Nespresso Machine", "Mini Bar", "Complimentary Breakfast", "Luxury Bathrobe"}) {}

    [[nodiscard]] std::string getTypeName() const override {
        return "Deluxe Room";
    }

    [[nodiscard]] std::string getDescription() const override {
        return "Spacious room featuring panoramic views, plush King bed, mini-bar, and gourmet coffee machine.";
    }

    [[nodiscard]] double getServiceMultiplier() const override {
        return 1.1; // 10% premium on specialty services
    }
};

} // namespace hotel::models
