cmake_minimum_required(VERSION 3.16)
project(HotelManagementSystem
    VERSION 1.0.0
    DESCRIPTION "Enterprise Hotel Property Management System in Modern C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Common library sources
set(CORE_SOURCES
    src/Models/Room.cpp
    src/Models/Guest.cpp
    src/Models/Reservation.cpp
    src/Services/StorageManager.cpp
    src/Services/HotelManager.cpp
    src/Utils/DateTimeUtils.cpp
    src/Utils/TerminalUI.cpp
)

add_library(hotel_core STATIC ${CORE_SOURCES})
target_include_directories(hotel_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Main executable
add_executable(hotel_system src/main.cpp)
target_link_libraries(hotel_system PRIVATE hotel_core)

# Unit tests executable
add_executable(hotel_tests tests/test_hotel.cpp)
target_link_libraries(hotel_tests PRIVATE hotel_core)

# Enable CTest
enable_testing()
add_test(NAME HotelSystemTests COMMAND hotel_tests)
