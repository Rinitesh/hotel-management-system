#include "Services/StorageManager.hpp"
#include "Patterns/RoomFactory.hpp"
#include "Utils/Exceptions.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <algorithm>
#include <iostream>

#if defined(_WIN32)
#include <direct.h>
#define MKDIR(d) _mkdir(d)
#else
#define MKDIR(d) mkdir(d, 0755)
#endif

namespace hotel::services {

StorageManager::StorageManager(std::string dataDir)
    : dataDir_(std::move(dataDir)) {
    ensureDirectoryExists();
}

std::string StorageManager::getRoomsFilePath() const {
    return dataDir_ + "/rooms.csv";
}

std::string StorageManager::getBookingsFilePath() const {
    return dataDir_ + "/bookings.csv";
}

std::string StorageManager::getGuestsFilePath() const {
    return dataDir_ + "/guests.csv";
}

void StorageManager::ensureDirectoryExists() {
    struct stat info{};
    if (stat(dataDir_.c_str(), &info) != 0) {
        MKDIR(dataDir_.c_str());
    }
}

std::vector<std::string> StorageManager::splitCsvLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                token += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            tokens.push_back(token);
            token.clear();
        } else {
            token += c;
        }
    }
    tokens.push_back(token);
    return tokens;
}

std::string StorageManager::escapeCsvField(const std::string& field) {
    if (field.find(',') != std::string::npos || field.find('"') != std::string::npos || field.find('\n') != std::string::npos) {
        std::string escaped = "\"";
        for (char c : field) {
            if (c == '"') escaped += "\"\"";
            else escaped += c;
        }
        escaped += "\"";
        return escaped;
    }
    return field;
}

void StorageManager::validateForeignKey(int roomNumber, const std::unordered_set<int>& validRoomNumbers) {
    if (validRoomNumbers.find(roomNumber) == validRoomNumbers.end()) {
        throw InvalidForeignKeyException("RoomNumber", std::to_string(roomNumber));
    }
}

void StorageManager::seedInitialDataIfEmpty() {
    std::ifstream rf(getRoomsFilePath());
    if (rf.good()) return; // Already exists

    // Seed default rooms in INR
    std::vector<std::unique_ptr<models::Room>> seedRooms;
    seedRooms.push_back(patterns::RoomFactory::createRoom(101, models::RoomType::Standard, 1800.0, models::RoomStatus::Available));
    seedRooms.push_back(patterns::RoomFactory::createRoom(102, models::RoomType::Standard, 2600.0, models::RoomStatus::Available));
    seedRooms.push_back(patterns::RoomFactory::createRoom(103, models::RoomType::Deluxe, 2600.0, models::RoomStatus::Occupied));
    seedRooms.push_back(patterns::RoomFactory::createRoom(104, models::RoomType::Deluxe, 4200.0, models::RoomStatus::Maintenance));
    seedRooms.push_back(patterns::RoomFactory::createRoom(105, models::RoomType::Suite, 4200.0, models::RoomStatus::Available));
    saveRooms(seedRooms);

    // Seed default bookings
    std::unordered_map<std::string, models::Reservation> seedBookings;
    models::Reservation b1("BKG-0001", "GUEST-0001", 103, "2026-11-01", 3, 2600.0, models::ReservationStatus::CheckedIn);
    seedBookings[b1.getBookingId()] = b1;

    models::Reservation b2("BKG-0002", "GUEST-0002", 101, "2026-11-05", 2, 1800.0, models::ReservationStatus::Confirmed);
    seedBookings[b2.getBookingId()] = b2;

    saveReservations(seedBookings);
}

std::vector<std::unique_ptr<models::Room>> StorageManager::loadRooms() {
    seedInitialDataIfEmpty();
    std::ifstream file(getRoomsFilePath());
    std::vector<std::unique_ptr<models::Room>> rooms;
    if (!file.is_open()) return rooms;

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        // Trim carriage return if present
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (isHeader) {
            isHeader = false;
            continue;
        }

        auto tokens = splitCsvLine(line);
        if (tokens.size() >= 4) {
            try {
                int roomNum = std::stoi(tokens[0]);
                models::RoomType type = models::stringToRoomType(tokens[1]);
                double nightlyRateINR = std::stod(tokens[2]);
                models::RoomStatus status = models::stringToRoomStatus(tokens[3]);

                auto room = patterns::RoomFactory::createRoom(roomNum, type, nightlyRateINR, status);
                rooms.push_back(std::move(room));
            } catch (...) {
                // Ignore malformed line
            }
        }
    }
    return rooms;
}

std::unordered_map<std::string, models::Reservation> StorageManager::loadReservations(
    const std::vector<std::unique_ptr<models::Room>>& rooms) {
    seedInitialDataIfEmpty();

    std::unordered_set<int> validRooms;
    std::unordered_map<int, double> roomRates;
    for (const auto& r : rooms) {
        validRooms.insert(r->getRoomNumber());
        roomRates[r->getRoomNumber()] = r->getBaseRate();
    }

    std::ifstream file(getBookingsFilePath());
    std::unordered_map<std::string, models::Reservation> reservations;
    if (!file.is_open()) return reservations;

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (isHeader) {
            isHeader = false;
            continue;
        }

        auto tokens = splitCsvLine(line);
        if (tokens.size() >= 6) {
            try {
                std::string bookingId = tokens[0];
                std::string customerId = tokens[1];
                int roomNum = std::stoi(tokens[2]);
                std::string checkIn = tokens[3];
                int nights = std::stoi(tokens[4]);
                models::ReservationStatus status = models::stringToReservationStatus(tokens[5]);

                // Foreign key validation
                validateForeignKey(roomNum, validRooms);

                double rate = roomRates.count(roomNum) ? roomRates[roomNum] : 2000.0;
                models::Reservation res(bookingId, customerId, roomNum, checkIn, nights, rate, status);
                reservations[bookingId] = res;
            } catch (const InvalidForeignKeyException& e) {
                // Preserving foreign key validation guarantee: log/rethrow if critical
                std::cerr << "Warning during import: " << e.what() << "\n";
            } catch (...) {
                // Skip malformed rows
            }
        }
    }
    return reservations;
}

std::unordered_map<std::string, models::Guest> StorageManager::loadGuests() {
    std::unordered_map<std::string, models::Guest> guests;
    std::ifstream file(getGuestsFilePath());
    if (file.is_open()) {
        std::string line;
        bool isHeader = true;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            if (isHeader) {
                isHeader = false;
                continue;
            }

            auto tokens = splitCsvLine(line);
            if (tokens.size() >= 8) {
                std::string id = tokens[0];
                std::string name = tokens[1];
                std::string email = tokens[2];
                std::string phone = tokens[3];
                std::string idProof = tokens[4];
                bool isVIP = (tokens[5] == "1" || tokens[5] == "true");
                int totalStays = std::stoi(tokens[6]);
                int points = std::stoi(tokens[7]);

                guests[id] = models::Guest(id, name, email, phone, idProof, isVIP, totalStays, points);
            }
        }
    }
    return guests;
}

void StorageManager::saveRooms(const std::vector<std::unique_ptr<models::Room>>& rooms) {
    ensureDirectoryExists();
    std::ofstream file(getRoomsFilePath());
    if (!file.is_open()) {
        throw StorageException("Failed to open " + getRoomsFilePath() + " for writing.");
    }

    file << "RoomNumber,Type,NightlyRateINR,Status\n";

    std::vector<const models::Room*> sortedRooms;
    sortedRooms.reserve(rooms.size());
    for (const auto& r : rooms) sortedRooms.push_back(r.get());
    std::sort(sortedRooms.begin(), sortedRooms.end(), [](const auto* a, const auto* b) {
        return a->getRoomNumber() < b->getRoomNumber();
    });

    for (const auto* room : sortedRooms) {
        file << room->getRoomNumber() << ","
             << escapeCsvField(models::roomTypeToString(room->getType())) << ","
             << static_cast<long long>(room->getBaseRate()) << ","
             << escapeCsvField(models::roomStatusToString(room->getStatus())) << "\n";
    }
}

void StorageManager::saveReservations(const std::unordered_map<std::string, models::Reservation>& reservations) {
    ensureDirectoryExists();
    std::ofstream file(getBookingsFilePath());
    if (!file.is_open()) {
        throw StorageException("Failed to open " + getBookingsFilePath() + " for writing.");
    }

    file << "BookingID,CustomerID,RoomNumber,CheckIn,Nights,Status\n";

    std::vector<models::Reservation> sortedBookings;
    sortedBookings.reserve(reservations.size());
    for (const auto& [_, res] : reservations) {
        sortedBookings.push_back(res);
    }
    std::sort(sortedBookings.begin(), sortedBookings.end(), [](const auto& a, const auto& b) {
        return a.getBookingId() < b.getBookingId();
    });

    for (const auto& b : sortedBookings) {
        file << escapeCsvField(b.getBookingId()) << ","
             << escapeCsvField(b.getCustomerId()) << ","
             << b.getRoomNumber() << ","
             << escapeCsvField(b.getCheckIn()) << ","
             << b.getNights() << ","
             << escapeCsvField(models::reservationStatusToString(b.getStatus())) << "\n";
    }
}

void StorageManager::saveGuests(const std::unordered_map<std::string, models::Guest>& guests) {
    ensureDirectoryExists();
    std::ofstream file(getGuestsFilePath());
    if (!file.is_open()) {
        throw StorageException("Failed to open " + getGuestsFilePath() + " for writing.");
    }

    file << "GuestId,Name,Email,Phone,IdProof,IsVIP,TotalStays,RewardPoints\n";
    for (const auto& [id, guest] : guests) {
        file << escapeCsvField(guest.getId()) << ","
             << escapeCsvField(guest.getName()) << ","
             << escapeCsvField(guest.getEmail()) << ","
             << escapeCsvField(guest.getPhone()) << ","
             << escapeCsvField(guest.getIdProof()) << ","
             << (guest.isVIP() ? "1" : "0") << ","
             << guest.getTotalStays() << ","
             << guest.getRewardPoints() << "\n";
    }
}

} // namespace hotel::services
