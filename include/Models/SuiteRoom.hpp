#pragma once

#include "Models/Room.hpp"

namespace hotel::models {

class SuiteRoom : public Room {
public:
    explicit SuiteRoom(int roomNumber, double baseRate = 280.0, RoomStatus status = RoomStatus::Available)
        : Room(roomNumber, RoomType::Suite, baseRate, 4, status,
               {"Separate Living Room", "Master Bedroom with King Bed", "Marble Jacuzzi", "Executive Lounge Access", "Cocktail Hour", "24/7 Room Service"}) {}

    [[nodiscard]] std::string getTypeName() const override {
        return "Executive Suite";
    }

    [[nodiscard]] std::string getDescription() const override {
        return "Upscale multi-room suite with private living lounge, marble bath jacuzzi, and VIP lounge privileges.";
    }

    [[nodiscard]] double getServiceMultiplier() const override {
        return 1.2;
    }
};

class PresidentialSuite : public Room {
public:
    explicit PresidentialSuite(int roomNumber, double baseRate = 650.0, RoomStatus status = RoomStatus::Available)
        : Room(roomNumber, RoomType::Presidential, baseRate, 6, status,
               {"Penthouse Floor", "Panoramic Skyline View", "Private Terrace & Plunge Pool", "Dedicated Butler Service", "Private Dining Room", "Chauffeur Airport Transfer"}) {}

    [[nodiscard]] std::string getTypeName() const override {
        return "Presidential Suite";
    }

    [[nodiscard]] std::string getDescription() const override {
        return "The pinnacle of luxury: top-floor penthouse with private plunge pool, 24/7 dedicated butler, and bespoke dining.";
    }

    [[nodiscard]] double getServiceMultiplier() const override {
        return 1.5;
    }
};

} // namespace hotel::models
