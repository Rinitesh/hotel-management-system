#pragma once

#include <string>
#include <vector>
#include "Models/ServiceItem.hpp"

namespace hotel::models {

enum class ReservationStatus {
    Confirmed,
    CheckedIn,
    CheckedOut,
    Cancelled
};

std::string reservationStatusToString(ReservationStatus status);
ReservationStatus stringToReservationStatus(const std::string& str);

class Reservation {
public:
    Reservation() = default;
    Reservation(std::string reservationId, std::string guestId, int roomNumber,
                std::string checkInDate, std::string checkOutDate, int numberOfNights,
                double roomRatePerNight, ReservationStatus status = ReservationStatus::Confirmed,
                std::string createdAt = "");

    // Getters
    [[nodiscard]] const std::string& getReservationId() const { return reservationId_; }
    [[nodiscard]] const std::string& getGuestId() const { return guestId_; }
    [[nodiscard]] int getRoomNumber() const { return roomNumber_; }
    [[nodiscard]] const std::string& getCheckInDate() const { return checkInDate_; }
    [[nodiscard]] const std::string& getCheckOutDate() const { return checkOutDate_; }
    [[nodiscard]] int getNumberOfNights() const { return numberOfNights_; }
    [[nodiscard]] double getRoomRatePerNight() const { return roomRatePerNight_; }
    [[nodiscard]] ReservationStatus getStatus() const { return status_; }
    [[nodiscard]] const std::vector<ServiceItem>& getServices() const { return services_; }
    [[nodiscard]] double getRoomTotal() const { return roomTotal_; }
    [[nodiscard]] double getServicesTotal() const { return servicesTotal_; }
    [[nodiscard]] double getTaxAmount() const { return taxAmount_; }
    [[nodiscard]] double getDiscountAmount() const { return discountAmount_; }
    [[nodiscard]] double getGrandTotal() const { return grandTotal_; }
    [[nodiscard]] bool isPaid() const { return isPaid_; }
    [[nodiscard]] const std::string& getPaymentMethod() const { return paymentMethod_; }
    [[nodiscard]] const std::string& getCreatedAt() const { return createdAt_; }

    // Folio operations
    void addService(const ServiceItem& item);
    void recalculateTotals(double discountPct = 0.0, double taxRate = 0.12);

    // Lifecycle
    void setStatus(ReservationStatus newStatus) { status_ = newStatus; }
    void checkIn();
    void checkOut(const std::string& paymentMethod);
    void cancel();

    // Direct setters for deserialization
    void setFinancials(double roomTotal, double servicesTotal, double tax, double discount, double grandTotal, bool paid, std::string method);

private:
    std::string reservationId_;
    std::string guestId_;
    int roomNumber_ = 0;
    std::string checkInDate_;
    std::string checkOutDate_;
    int numberOfNights_ = 1;
    double roomRatePerNight_ = 0.0;
    ReservationStatus status_ = ReservationStatus::Confirmed;
    std::string createdAt_;

    std::vector<ServiceItem> services_;
    double roomTotal_ = 0.0;
    double servicesTotal_ = 0.0;
    double taxAmount_ = 0.0;
    double discountAmount_ = 0.0;
    double grandTotal_ = 0.0;
    bool isPaid_ = false;
    std::string paymentMethod_ = "Pending";
};

} // namespace hotel::models
