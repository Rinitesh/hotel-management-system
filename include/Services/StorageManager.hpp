#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Models/Room.hpp"
#include "Models/Guest.hpp"
#include "Models/Reservation.hpp"

namespace hotel::services {

class StorageManager {
public:
    explicit StorageManager(std::string dataDir = "data");

    // File paths
    std::string getRoomsFilePath() const;
    std::string getGuestsFilePath() const;
    std::string getReservationsFilePath() const;

    // Load data (creates defaults if files do not exist)
    std::vector<std::unique_ptr<models::Room>> loadRooms();
    std::unordered_map<std::string, models::Guest> loadGuests();
    std::unordered_map<std::string, models::Reservation> loadReservations();

    // Save data
    void saveRooms(const std::vector<std::unique_ptr<models::Room>>& rooms);
    void saveGuests(const std::unordered_map<std::string, models::Guest>& guests);
    void saveReservations(const std::unordered_map<std::string, models::Reservation>& reservations);

    // Initial seed generation
    void seedInitialDataIfEmpty();

private:
    std::string dataDir_;

    void ensureDirectoryExists();
    static std::vector<std::string> splitCsvLine(const std::string& line);
    static std::string escapeCsvField(const std::string& field);
};

} // namespace hotel::services
