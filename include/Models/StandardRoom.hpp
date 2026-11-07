#pragma once

#include "Models/Room.hpp"

namespace hotel::models {

class StandardRoom : public Room {
public:
    explicit StandardRoom(int roomNumber, double baseRate = 80.0, RoomStatus status = RoomStatus::Available)
        : Room(roomNumber, RoomType::Standard, baseRate, 2, status,
               {"High-Speed Wi-Fi", "HD Smart TV", "Air Conditioning", "Ensuite Shower", "Work Desk"}) {}

    [[nodiscard]] std::string getTypeName() const override {
        return "Standard Room";
    }

    [[nodiscard]] std::string getDescription() const override {
        return "Comfortable room with Queen/Twin bedding, essential amenities, and dedicated workspace.";
    }

    [[nodiscard]] double getServiceMultiplier() const override {
        return 1.0;
    }
};

} // namespace hotel::models
