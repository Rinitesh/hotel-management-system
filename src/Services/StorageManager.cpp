#include "Services/StorageManager.hpp"
#include "Patterns/RoomFactory.hpp"
#include "Utils/Exceptions.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
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

std::string StorageManager::getGuestsFilePath() const {
    return dataDir_ + "/guests.csv";
}

std::string StorageManager::getReservationsFilePath() const {
    return dataDir_ + "/reservations.csv";
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

void StorageManager::seedInitialDataIfEmpty() {
    std::ifstream rf(getRoomsFilePath());
    if (rf.good()) return; // Already exists

    // Seed Rooms
    std::vector<std::unique_ptr<models::Room>> seedRooms;
    seedRooms.push_back(patterns::RoomFactory::createRoom(101, models::RoomType::Standard, 80.0, models::RoomStatus::Available));
    seedRooms.push_back(patterns::RoomFactory::createRoom(102, models::RoomType::Standard, 80.0, models::RoomStatus::Available));
    seedRooms.push_back(patterns::RoomFactory::createRoom(103, models::RoomType::Standard, 85.0, models::RoomStatus::Occupied));
    seedRooms.push_back(patterns::RoomFactory::createRoom(104, models::RoomType::Standard, 80.0, models::RoomStatus::Maintenance));

    seedRooms.push_back(patterns::RoomFactory::createRoom(201, models::RoomType::Deluxe, 150.0, models::RoomStatus::Available));
    seedRooms.push_back(patterns::RoomFactory::createRoom(202, models::RoomType::Deluxe, 150.0, models::RoomStatus::Occupied));
    seedRooms.push_back(patterns::RoomFactory::createRoom(203, models::RoomType::Deluxe, 160.0, models::RoomStatus::Reserved));
    seedRooms.push_back(patterns::RoomFactory::createRoom(204, models::RoomType::Deluxe, 150.0, models::RoomStatus::Available));

    seedRooms.push_back(patterns::RoomFactory::createRoom(301, models::RoomType::Suite, 280.0, models::RoomStatus::Available));
    seedRooms.push_back(patterns::RoomFactory::createRoom(302, models::RoomType::Suite, 290.0, models::RoomStatus::Occupied));

    seedRooms.push_back(patterns::RoomFactory::createRoom(401, models::RoomType::Presidential, 650.0, models::RoomStatus::Available));
    seedRooms.push_back(patterns::RoomFactory::createRoom(402, models::RoomType::Presidential, 700.0, models::RoomStatus::Reserved));
    saveRooms(seedRooms);

    // Seed Guests
    std::unordered_map<std::string, models::Guest> seedGuests;
    seedGuests["GST-1001"] = models::Guest("GST-1001", "Alice Morgan", "alice.m@example.com", "+1-555-0101", "PASS-US-9281", false, 1, 200);
    seedGuests["GST-1002"] = models::Guest("GST-1002", "David Chen", "david.chen@techcorp.io", "+1-555-0142", "DL-CA-449102", true, 5, 1850);
    seedGuests["GST-1003"] = models::Guest("GST-1003", "Sophia Rodriguez", "sophia.r@worldtravel.org", "+44-7700-900321", "PASS-UK-8831", true, 4, 1200);
    seedGuests["GST-1004"] = models::Guest("GST-1004", "Michael Scott", "michael.s@dunder.com", "+1-555-0199", "DL-PA-192837", false, 1, 100);
    saveGuests(seedGuests);

    // Seed Reservations
    std::unordered_map<std::string, models::Reservation> seedReservations;

    models::Reservation res1("RES-20261101-1001", "GST-1002", 202, "2026-11-05", "2026-11-09", 4, 150.0, models::ReservationStatus::CheckedIn, "2026-11-01 10:30:00");
    res1.addService(models::ServiceItem("SRV-1", "Executive Buffet Breakfast", 35.0, "Dining", "2026-11-06 08:30"));
    res1.addService(models::ServiceItem("SRV-2", "Airport Luxury Shuttle", 50.0, "Transport", "2026-11-05 14:00"));
    res1.recalculateTotals(15.0, 0.12); // VIP discount 15%
    seedReservations[res1.getReservationId()] = res1;

    models::Reservation res2("RES-20261102-1002", "GST-1003", 302, "2026-11-06", "2026-11-10", 4, 290.0, models::ReservationStatus::CheckedIn, "2026-11-02 16:15:00");
    res2.addService(models::ServiceItem("SRV-3", "Aromatherapy Full Spa Massage", 120.0, "Spa", "2026-11-07 11:00"));
    res2.recalculateTotals(15.0, 0.12);
    seedReservations[res2.getReservationId()] = res2;

    models::Reservation res3("RES-20261104-1003", "GST-1001", 103, "2026-11-07", "2026-11-09", 2, 85.0, models::ReservationStatus::CheckedIn, "2026-11-04 11:00:00");
    res3.recalculateTotals(0.0, 0.12);
    seedReservations[res3.getReservationId()] = res3;

    models::Reservation res4("RES-20261105-1004", "GST-1004", 402, "2026-11-12", "2026-11-15", 3, 700.0, models::ReservationStatus::Confirmed, "2026-11-05 09:20:00");
    res4.recalculateTotals(0.0, 0.12);
    seedReservations[res4.getReservationId()] = res4;

    saveReservations(seedReservations);
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
        if (isHeader) {
            isHeader = false;
            continue;
        }

        auto tokens = splitCsvLine(line);
        if (tokens.size() >= 4) {
            int roomNum = std::stoi(tokens[0]);
            models::RoomType type = models::stringToRoomType(tokens[1]);
            double baseRate = std::stod(tokens[2]);
            models::RoomStatus status = models::stringToRoomStatus(tokens[3]);

            auto room = patterns::RoomFactory::createRoom(roomNum, type, baseRate, status);
            rooms.push_back(std::move(room));
        }
    }
    return rooms;
}

std::unordered_map<std::string, models::Guest> StorageManager::loadGuests() {
    seedInitialDataIfEmpty();
    std::ifstream file(getGuestsFilePath());
    std::unordered_map<std::string, models::Guest> guests;
    if (!file.is_open()) return guests;

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
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
    return guests;
}

std::unordered_map<std::string, models::Reservation> StorageManager::loadReservations() {
    seedInitialDataIfEmpty();
    std::ifstream file(getReservationsFilePath());
    std::unordered_map<std::string, models::Reservation> reservations;
    if (!file.is_open()) return reservations;

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (isHeader) {
            isHeader = false;
            continue;
        }

        auto tokens = splitCsvLine(line);
        if (tokens.size() >= 17) {
            std::string resId = tokens[0];
            std::string guestId = tokens[1];
            int roomNum = std::stoi(tokens[2]);
            std::string checkIn = tokens[3];
            std::string checkOut = tokens[4];
            int nights = std::stoi(tokens[5]);
            double roomRate = std::stod(tokens[6]);
            models::ReservationStatus status = models::stringToReservationStatus(tokens[7]);
            std::string createdAt = tokens[8];

            models::Reservation res(resId, guestId, roomNum, checkIn, checkOut, nights, roomRate, status, createdAt);

            double roomTot = std::stod(tokens[9]);
            double srvTot = std::stod(tokens[10]);
            double tax = std::stod(tokens[11]);
            double disc = std::stod(tokens[12]);
            double grandTot = std::stod(tokens[13]);
            bool isPaid = (tokens[14] == "1" || tokens[14] == "true");
            std::string paymentMethod = tokens[15];

            std::string srvSerial = tokens[16];
            if (!srvSerial.empty()) {
                std::istringstream srvStream(srvSerial);
                std::string srvToken;
                while (std::getline(srvStream, srvToken, ';')) {
                    if (srvToken.empty()) continue;
                    std::istringstream itemStream(srvToken);
                    std::string sid, sname, sprice, scat, sts;
                    std::getline(itemStream, sid, '|');
                    std::getline(itemStream, sname, '|');
                    std::getline(itemStream, sprice, '|');
                    std::getline(itemStream, scat, '|');
                    std::getline(itemStream, sts, '|');
                    if (!sid.empty() && !sprice.empty()) {
                        res.addService(models::ServiceItem(sid, sname, std::stod(sprice), scat, sts));
                    }
                }
            }

            res.setFinancials(roomTot, srvTot, tax, disc, grandTot, isPaid, paymentMethod);
            reservations[resId] = res;
        }
    }
    return reservations;
}

void StorageManager::saveRooms(const std::vector<std::unique_ptr<models::Room>>& rooms) {
    ensureDirectoryExists();
    std::ofstream file(getRoomsFilePath());
    if (!file.is_open()) {
        throw StorageException("Failed to open " + getRoomsFilePath() + " for writing.");
    }

    file << "RoomNumber,Type,BaseRate,Status\n";
    for (const auto& room : rooms) {
        file << room->getRoomNumber() << ","
             << escapeCsvField(models::roomTypeToString(room->getType())) << ","
             << room->getBaseRate() << ","
             << escapeCsvField(models::roomStatusToString(room->getStatus())) << "\n";
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

void StorageManager::saveReservations(const std::unordered_map<std::string, models::Reservation>& reservations) {
    ensureDirectoryExists();
    std::ofstream file(getReservationsFilePath());
    if (!file.is_open()) {
        throw StorageException("Failed to open " + getReservationsFilePath() + " for writing.");
    }

    file << "ReservationId,GuestId,RoomNumber,CheckInDate,CheckOutDate,NumberOfNights,RoomRatePerNight,"
         << "Status,CreatedAt,RoomTotal,ServicesTotal,TaxAmount,DiscountAmount,GrandTotal,IsPaid,PaymentMethod,Services\n";

    for (const auto& [id, res] : reservations) {
        std::string srvSerial;
        for (const auto& s : res.getServices()) {
            if (!srvSerial.empty()) srvSerial += ";";
            srvSerial += s.getId() + "|" + s.getName() + "|" + std::to_string(s.getPrice()) + "|" + s.getCategory() + "|" + s.getTimestamp();
        }

        file << escapeCsvField(res.getReservationId()) << ","
             << escapeCsvField(res.getGuestId()) << ","
             << res.getRoomNumber() << ","
             << escapeCsvField(res.getCheckInDate()) << ","
             << escapeCsvField(res.getCheckOutDate()) << ","
             << res.getNumberOfNights() << ","
             << res.getRoomRatePerNight() << ","
             << escapeCsvField(models::reservationStatusToString(res.getStatus())) << ","
             << escapeCsvField(res.getCreatedAt()) << ","
             << res.getRoomTotal() << ","
             << res.getServicesTotal() << ","
             << res.getTaxAmount() << ","
             << res.getDiscountAmount() << ","
             << res.getGrandTotal() << ","
             << (res.isPaid() ? "1" : "0") << ","
             << escapeCsvField(res.getPaymentMethod()) << ","
             << escapeCsvField(srvSerial) << "\n";
    }
}

} // namespace hotel::services
