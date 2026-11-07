#include "Models/Reservation.hpp"
#include "Utils/Exceptions.hpp"
#include <algorithm>
#include <numeric>
#include <utility>

namespace hotel::models {

std::string reservationStatusToString(ReservationStatus status) {
    switch (status) {
        case ReservationStatus::Confirmed: return "Confirmed";
        case ReservationStatus::CheckedIn: return "CheckedIn";
        case ReservationStatus::CheckedOut: return "CheckedOut";
        case ReservationStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

ReservationStatus stringToReservationStatus(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    if (s == "checkedin" || s == "checked-in" || s == "checked_in") return ReservationStatus::CheckedIn;
    if (s == "checkedout" || s == "checked-out" || s == "checked_out") return ReservationStatus::CheckedOut;
    if (s == "cancelled" || s == "canceled") return ReservationStatus::Cancelled;
    return ReservationStatus::Confirmed;
}

Reservation::Reservation(std::string reservationId, std::string guestId, int roomNumber,
                         std::string checkInDate, std::string checkOutDate, int numberOfNights,
                         double roomRatePerNight, ReservationStatus status, std::string createdAt)
    : reservationId_(std::move(reservationId)),
      guestId_(std::move(guestId)),
      roomNumber_(roomNumber),
      checkInDate_(std::move(checkInDate)),
      checkOutDate_(std::move(checkOutDate)),
      numberOfNights_(std::max(1, numberOfNights)),
      roomRatePerNight_(roomRatePerNight),
      status_(status),
      createdAt_(std::move(createdAt)) {
    recalculateTotals(0.0, 0.12);
}

void Reservation::addService(const ServiceItem& item) {
    if (status_ == ReservationStatus::Cancelled || status_ == ReservationStatus::CheckedOut) {
        throw InvalidOperationException("Cannot add services to a cancelled or checked-out reservation.");
    }
    services_.push_back(item);
    recalculateTotals();
}

void Reservation::recalculateTotals(double discountPct, double taxRate) {
    roomTotal_ = roomRatePerNight_ * static_cast<double>(numberOfNights_);

    servicesTotal_ = std::accumulate(services_.begin(), services_.end(), 0.0,
        [](double sum, const ServiceItem& item) {
            return sum + item.getPrice();
        });

    double subtotal = roomTotal_ + servicesTotal_;
    discountAmount_ = subtotal * (discountPct / 100.0);
    double taxable = std::max(0.0, subtotal - discountAmount_);
    taxAmount_ = taxable * taxRate;
    grandTotal_ = taxable + taxAmount_;
}

void Reservation::checkIn() {
    if (status_ != ReservationStatus::Confirmed) {
        throw InvalidOperationException("Reservation cannot be checked in from current status: " + reservationStatusToString(status_));
    }
    status_ = ReservationStatus::CheckedIn;
}

void Reservation::checkOut(const std::string& paymentMethod) {
    if (status_ != ReservationStatus::CheckedIn) {
        throw InvalidOperationException("Only checked-in reservations can be checked out.");
    }
    status_ = ReservationStatus::CheckedOut;
    isPaid_ = true;
    paymentMethod_ = paymentMethod;
}

void Reservation::cancel() {
    if (status_ == ReservationStatus::CheckedOut) {
        throw InvalidOperationException("Cannot cancel a completed (checked out) reservation.");
    }
    status_ = ReservationStatus::Cancelled;
}

void Reservation::setFinancials(double roomTotal, double servicesTotal, double tax,
                                double discount, double grandTotal, bool paid, std::string method) {
    roomTotal_ = roomTotal;
    servicesTotal_ = servicesTotal;
    taxAmount_ = tax;
    discountAmount_ = discount;
    grandTotal_ = grandTotal;
    isPaid_ = paid;
    paymentMethod_ = std::move(method);
}

} // namespace hotel::models
