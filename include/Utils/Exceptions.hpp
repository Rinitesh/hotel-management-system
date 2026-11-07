#pragma once

#include <stdexcept>
#include <string>

namespace hotel {

class HotelException : public std::runtime_error {
public:
    explicit HotelException(const std::string& message)
        : std::runtime_error(message) {}
};

class RoomNotFoundException : public HotelException {
public:
    explicit RoomNotFoundException(int roomNumber)
        : HotelException("Room " + std::to_string(roomNumber) + " was not found in the system.") {}
};

class RoomNotAvailableException : public HotelException {
public:
    explicit RoomNotAvailableException(int roomNumber, const std::string& currentStatus)
        : HotelException("Room " + std::to_string(roomNumber) + " is currently " + currentStatus + " and cannot be booked.") {}
};

class GuestNotFoundException : public HotelException {
public:
    explicit GuestNotFoundException(const std::string& guestId)
        : HotelException("Guest with ID '" + guestId + "' was not found.") {}
};

class ReservationNotFoundException : public HotelException {
public:
    explicit ReservationNotFoundException(const std::string& reservationId)
        : HotelException("Reservation with ID '" + reservationId + "' was not found.") {}
};

class InvalidOperationException : public HotelException {
public:
    explicit InvalidOperationException(const std::string& message)
        : HotelException("Invalid Operation: " + message) {}
};

class InvalidForeignKeyException : public HotelException {
public:
    explicit InvalidForeignKeyException(const std::string& field, const std::string& value)
        : HotelException("Foreign Key Constraint Violation: " + field + " '" + value + "' does not exist.") {}
};

class StorageException : public HotelException {
public:
    explicit StorageException(const std::string& message)
        : HotelException("Storage/IO Error: " + message) {}
};

} // namespace hotel
