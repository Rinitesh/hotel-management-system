# Grand Horizon - Hotel Management System (Modern C++)

[![Language: C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![Build System: CMake](https://img.shields.io/badge/Build-CMake%203.16+-orange.svg?logo=cmake)](https://cmake.org)
[![Tests: 39 Passed](https://img.shields.io/badge/Tests-39%2F39%20Passing-brightgreen.svg)](#test-suite)
[![Platform: macOS / Linux / Windows](https://img.shields.io/badge/Platform-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey.svg)](#installation--build)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

An enterprise-grade, modular Hotel Property Management System (PMS) built from scratch in **Modern C++ (C++17)**. Designed with software engineering best practices, design patterns (Factory, Strategy, Repository), memory safety via RAII and smart pointers, persistent CSV data storage, and a rich, interactive ANSI terminal user interface.

---

## Table of Contents
- [System Architecture](#system-architecture)
- [Design Patterns & C++ Concepts](#design-patterns--c-concepts)
- [Key Features](#key-features)
- [Directory Structure](#directory-structure)
- [Installation & Build](#installation--build)
- [Running the Application](#running-the-application)
- [Test Suite](#test-suite)
- [Folio & Invoice Sample Output](#folio--invoice-sample-output)

---

## System Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                    Terminal UI / CLI Layer                   │
│   (Menu System, ANSI Color formatting, Input Validation)     │
└──────────────────────────────┬───────────────────────────────┘
                               │
┌──────────────────────────────▼───────────────────────────────┐
│                    HotelManager (Facade)                     │
│  • Orchestration • Lifecycle Management • Business Rules     │
└───────┬──────────────────────┬──────────────────────┬────────┘
        │                      │                      │
┌───────▼────────┐     ┌───────▼────────┐     ┌───────▼────────┐
│  RoomFactory   │     │ PricingStrategy│     │ StorageManager │
│  (Factory Mth) │     │ (Strategy Mth) │     │  (Persistence) │
└───────┬────────┘     └───────┬────────┘     └───────┬────────┘
        │                      │                      │
┌───────▼────────┐     ┌───────▼────────┐     ┌───────▼────────┐
│  Room Hierarchy│     │  Reservation & │     │ CSV Databases  │
│  • Standard    │     │  Guest Folios  │     │ • rooms.csv    │
│  • Deluxe      │     │  • Line items  │     │ • guests.csv   │
│  • Suite       │     │  • Tax & disc  │     │ • reservations │
│  • Presidential│     │  • Loyalty pts │     └────────────────┘
└────────────────┘     └────────────────┘
```

---

## Design Patterns & C++ Concepts

This project was built specifically to showcase clean, production-level C++ engineering:

1. **Factory Method Pattern (`RoomFactory`)**:
   - Decouples room instantiation from client code.
   - Cleanly constructs concrete room types (`StandardRoom`, `DeluxeRoom`, `SuiteRoom`, `PresidentialSuite`) based on `RoomType` enums.
2. **Strategy Pattern (`PricingStrategy`)**:
   - Dynamic rate calculation decoupled from room models.
   - Includes `StandardPricingStrategy`, `VIPPricingStrategy` (15% loyalty discount), `PeakSeasonPricingStrategy` (25% surcharge), and `ExtendedStayPricingStrategy` (7+ nights discount).
3. **Object-Oriented Design & Polymorphism**:
   - Abstract base class `Room` with virtual interfaces, `override` specifiers, and custom service multipliers.
4. **Modern Memory Safety (RAII & Smart Pointers)**:
   - Zero raw `new` / `delete`.
   - Polymorphic collections managed with `std::unique_ptr<Room>` and `std::vector`.
5. **Modern Standard Library Utilization**:
   - `<chrono>` and `<ctime>` for date arithmetic, duration calculation, and automated formatting.
   - `std::optional` for safe lookups without null-pointer hazards.
   - STL algorithms (`std::accumulate`, `std::find_if`, `std::transform`, `std::sort`).
   - Exception handling hierarchy derived from `std::runtime_error` (`RoomNotAvailableException`, `ReservationNotFoundException`, etc.).

---

## Key Features

* **Room Inventory Management**:
  * Live room tracking with status states: `Available`, `Occupied`, `Reserved`, `Maintenance`.
  * Multi-tier categories: Standard, Deluxe, Executive Suite, and Presidential Penthouse.
* **Guest CRM & VIP Loyalty Rewards**:
  * Profiles with contact details, government ID verification, and stay history.
  * Automated VIP tier progression based on visit counts and reward points accumulated.
* **Reservation Lifecycle**:
  * Date validation & collision prevention (prevents double bookings).
  * Check-In, Check-Out, and Reservation cancellation workflows.
* **Ancillary Room Services**:
  * In-house dining, spa passes, laundry service, airport chauffeur transfers.
  * Add services to room folio at any time during stay with itemized timestamps.
* **Billing & Invoice Generation**:
  * Real-time folio recalculation: Room rate × nights + ancillary services - discounts + 12% hotel tax.
  * Multi-channel payment recording (Credit Card, Debit Card, UPI, Cash, Corporate).
* **Operational KPI Dashboard**:
  * Occupancy rate percentage, occupied vs. available room counter, realized revenue tracking.
* **Automatic Seed Data**:
  * Pre-loaded with realistic rooms, active guests, and reservations out-of-the-box.

---

## Directory Structure

```
.
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # Comprehensive documentation
├── .gitignore                  # Git ignore rules
├── include/                    # Header files (.hpp)
│   ├── Models/
│   │   ├── Room.hpp            # Abstract Room base class
│   │   ├── StandardRoom.hpp    # Standard room implementation
│   │   ├── DeluxeRoom.hpp      # Deluxe room implementation
│   │   ├── SuiteRoom.hpp       # Suite & Presidential implementations
│   │   ├── Guest.hpp           # Guest entity with loyalty system
│   │   ├── ServiceItem.hpp     # Room service / amenity model
│   │   └── Reservation.hpp     # Reservation & billing folio
│   ├── Patterns/
│   │   ├── RoomFactory.hpp     # Factory Method design pattern
│   │   └── PricingStrategy.hpp # Strategy design pattern
│   ├── Services/
│   │   ├── HotelManager.hpp    # Core business orchestration facade
│   │   └── StorageManager.hpp  # CSV serialization & persistence
│   └── Utils/
│       ├── DateTimeUtils.hpp   # Date math, validation, and ID generator
│       ├── Exceptions.hpp      # Custom exception hierarchy
│       └── TerminalUI.hpp      # ANSI terminal rendering & input validation
├── src/                        # Implementation files (.cpp)
│   ├── Models/
│   │   ├── Room.cpp
│   │   ├── Guest.cpp
│   │   └── Reservation.cpp
│   ├── Services/
│   │   ├── HotelManager.cpp
│   │   └── StorageManager.cpp
│   ├── Utils/
│   │   ├── DateTimeUtils.cpp
│   │   └── TerminalUI.cpp
│   └── main.cpp                # Interactive CLI application
├── tests/
│   └── test_hotel.cpp          # Comprehensive unit test suite (39 tests)
└── data/                       # Persistent database files
    ├── rooms.csv
    ├── guests.csv
    └── reservations.csv
```

---

## Installation & Build

### Prerequisites
- C++17 compatible compiler:
  - **Clang** 5.0+ (macOS / Linux)
  - **GCC** 7.0+ (Linux)
  - **MSVC** 2017+ (Windows)
- **CMake** 3.16 or newer

### Building from Source

```bash
# Clone the repository
git clone https://github.com/Rinitesh/hotel-management-system.git
cd hotel-management-system

# Configure with CMake
cmake -B build

# Compile all targets
cmake --build build
```

---

## Running the Application

After building, run the main interactive CLI:

```bash
./build/hotel_system
```

### Interactive Main Menu
```
  ┌───────────────────────────────────────────────────────────────┐
  │         ★  GRAND HORIZON LUXURY RESORT & HOTEL  ★             │
  │              Enterprise Property Management System            │
  └───────────────────────────────────────────────────────────────┘

MAIN CONTROL PANEL
  [1]  View All Rooms & Live Inventory
  [2]  Book a Room (New Reservation)
  [3]  Guest Check-In
  [4]  Order Room Service & Amenities
  [5]  Guest Check-Out & Billing Folio
  [6]  View / Print Folio Invoice
  [7]  Cancel Reservation
  [8]  Guest Directory & Profiles
  [9]  Register New Guest
  [10] Add New Room (Admin)
  [11] Executive KPI & Financial Analytics
  [0]  Save & Exit
```

---

## Test Suite

A standalone test executable is included to guarantee correctness across all subsystems:

```bash
# Run unit tests directly
./build/hotel_tests

# Or run via CTest
ctest --test-dir build --output-on-failure
```

### Test Coverage Highlights:
- **DateTimeUtils**: Calendar validation, leap years, day delta calculation.
- **Factory & Polymorphism**: Correct object construction, virtual dispatch, pricing multipliers.
- **Pricing Strategies**: Standard, VIP discount (15%), Peak surcharge (25%), Extended stay (10%).
- **Guest & Loyalty**: Automatic tier upgrading after stay thresholds and point milestones.
- **Reservation Lifecycle**: State transitions (`Confirmed` -> `CheckedIn` -> `CheckedOut`), service additions, tax calculations.
- **Exception Safety**: Verifying `RoomNotAvailableException` on double booking attempts.

---

## Folio & Invoice Sample Output

```
========================================================================
                      GRAND HORIZON HOTEL & SUITES                      
                       OFFICIAL FOLIO & INVOICE                         
========================================================================
 Reservation ID : RES-20261101-1001
 Booking Date   : 2026-11-01 10:30:00
 Invoice Date   : 2026-11-07 13:15:22
 Status         : CheckedOut
 Payment Status : PAID via Credit Card
------------------------------------------------------------------------
 GUEST DETAILS:
   Name         : David Chen [VIP MEMBER]
   Email        : david.chen@techcorp.io
   Phone        : +1-555-0142
   ID / Passport: DL-CA-449102
------------------------------------------------------------------------
 ROOM ACCOMMODATION:
   Room Number  : Room 202 (Deluxe Room)
   Stay Period  : 2026-11-05 to 2026-11-09 (4 nights)
   Nightly Rate : $150.00
   Room Subtotal: $600.00
------------------------------------------------------------------------
 ANCILLARY SERVICES & ROOM CHARGES:
   1. [Dining]    Executive Buffet Breakfast        $   35.00
   2. [Transport] Airport Luxury Shuttle            $   50.00
   Services Subtotal: $85.00
------------------------------------------------------------------------
 FINANCIAL SUMMARY:
   Gross Total       : $    685.00
   Discount Applied  : -$    102.75
   Tax & Surchg (12%): $     69.87
   -----------------------------------------
   NET AMOUNT DUE    : $    652.12
========================================================================
               Thank you for staying at Grand Horizon!                  
========================================================================
```

---

## License

This project is open-source and available under the [MIT License](LICENSE).
