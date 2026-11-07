#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "Models/Room.hpp"
#include "Models/Guest.hpp"
#include "Models/Reservation.hpp"

namespace hotel::services {

class StorageManager {
public:
    explicit StorageManager(std::string dataDir = "data");

    // File paths
    [[nodiscard]] std::string getRoomsFilePath() const;
    [[nodiscard]] std::string getBookingsFilePath() const;
    [[nodiscard]] std::string getReservationsFilePath() const { return getBookingsFilePath(); }
    [[nodiscard]] std::string getGuestsFilePath() const;

    // Load data
    std::vector<std::unique_ptr<models::Room>> loadRooms();
    std::unordered_map<std::string, models::Reservation> loadReservations(const std::vector<std::unique_ptr<models::Room>>& rooms);
    std::unordered_map<std::string, models::Guest> loadGuests();

    // Save data
    void saveRooms(const std::vector<std::unique_ptr<models::Room>>& rooms);
    void saveReservations(const std::unordered_map<std::string, models::Reservation>& reservations);
    void saveGuests(const std::unordered_map<std::string, models::Guest>& guests);

    // Initial seed generation if missing
    void seedInitialDataIfEmpty();

    // Validation helper
    static void validateForeignKey(int roomNumber, const std::unordered_set<int>& validRoomNumbers);

private:
    std::string dataDir_;

    void ensureDirectoryExists();
    static std::vector<std::string> splitCsvLine(const std::string& line);
    static std::string escapeCsvField(const std::string& field);
};

} // namespace hotel::services
