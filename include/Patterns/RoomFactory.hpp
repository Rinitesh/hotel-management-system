#pragma once

#include <memory>
#include "Models/Room.hpp"
#include "Models/StandardRoom.hpp"
#include "Models/DeluxeRoom.hpp"
#include "Models/SuiteRoom.hpp"

namespace hotel::patterns {

class RoomFactory {
public:
    static std::unique_ptr<models::Room> createRoom(
        int roomNumber,
        models::RoomType type,
        double customRate = -1.0,
        models::RoomStatus status = models::RoomStatus::Available)
    {
        std::unique_ptr<models::Room> room;
        switch (type) {
            case models::RoomType::Standard:
                room = std::make_unique<models::StandardRoom>(roomNumber, (customRate > 0 ? customRate : 80.0), status);
                break;
            case models::RoomType::Deluxe:
                room = std::make_unique<models::DeluxeRoom>(roomNumber, (customRate > 0 ? customRate : 150.0), status);
                break;
            case models::RoomType::Suite:
                room = std::make_unique<models::SuiteRoom>(roomNumber, (customRate > 0 ? customRate : 280.0), status);
                break;
            case models::RoomType::Presidential:
                room = std::make_unique<models::PresidentialSuite>(roomNumber, (customRate > 0 ? customRate : 650.0), status);
                break;
        }
        return room;
    }
};

} // namespace hotel::patterns
