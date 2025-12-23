# Smart Hotel System (C++)
---

A console-based **hotel management system** written in C++ that manages **customers**, **reservations**, and **rooms**, with **JSON persistence**, **date validation**, a simple **loyalty discount**, and an **admin panel** for operational actions.

---
## Contributors
- **Lana Fliti** — [@Lana-fl](https://github.com/Lana-fl)
- **Aline Diab** — [@alinediab](https://github.com/alinediab)
- **Karim Nehme** — [@Karim-Nehme](https://github.com/Karim-Nehme)


## Table of Contents
1. [Features](#features)
   - [Customer Management](#customer-management)
   - [Reservation Management](#reservation-management)
   - [Room Management](#room-management)
   - [Scheduler](#scheduler)
   - [Invoice & Loyalty Discount](#invoice--loyalty-discount)
   - [Admin Panel](#admin-panel)
2. [Project Structure](#project-structure)
3. [Data Persistence](#data-persistence-json)
4. [Requirements](#requirements)
5. [Build & Run](#build--run)
6. [Usage](#usage)
7. [Admin Credentials](#admin-credentials)
8. [Notes](#notes--known-portability-detail)
9. [Possible Improvements](#possible-improvements)

---

## Features

### Customer Management
- Add, edit, delete customers
- List all customers
- Find a customer by ID and display their reservations
- Input validation:
  - Lebanese phone format validation (e.g., `70 123 456`)
  - Email validation (supports common domains + `.com`)

### Reservation Management
- Create reservations with:
  - Room type selection (`Single`, `Double`, `Suite`)
  - Check-in/check-out date validation (calendar-correct + cannot be in the past)
  - Automatic room assignment based on availability and room type
  - Night calculation and final price computation
- Cancel reservations (user-side)
- List all reservations + past/future listing (admin options)

### Room Management
- Hotel is initialized with fixed rooms:
  - Floor 1: `Single` rooms `101–112`
  - Floor 2: `Double` rooms `201–208`
  - Floor 3: `Suite` rooms `301–306`
- List available rooms
- List all rooms (with type, floor, price, and linked reservations)

### Scheduler
- Runs a scheduling pass to allocate reservations into rooms while avoiding overlaps.
- Clears and rebuilds room reservation links based on reservation data.

### Invoice + Loyalty Discount
- Prints an invoice after creating a reservation.
- Loyalty discount: **10% off** when a customer has **3+ bookings**.

### Admin Panel
- Cancel reservations
- Modify reservation price
- Add comments to rooms
- View all room comments
- List all / past / future reservations

---

## Project Structure

- `ProjectCP2.cpp`  
  Entry point (`main`) with the interactive menu.

- `HotelSystem.h / HotelSystem.cpp`  
  Core system logic: customers, rooms, reservations, persistence, admin menu.

- `Customer.h`  
  Customer entity + JSON serialization.

- `Reservation.h`  
  Reservation entity + JSON serialization + overlap check.

- `Room.h`  
  Room entity + JSON serialization + comments + linked reservations.

- `Scheduler.h / Scheduler.cpp`  
  Allocation logic to assign reservations to rooms without conflicts.

---

## Data Persistence (JSON)

The system automatically loads/saves data using JSON files in the program’s working directory:

- `customers.json`
- `reservations.json`
- `rooms.json`

> Tip: If you run the executable from a different folder, these files will be created/read there.

---

## Requirements

- C++17 compatible compiler (recommended)
- `nlohmann/json` single-header library (`json.hpp`)

Your code includes:
```cpp
#include "json.hpp"
```
So make sure json.hpp is available in the same directory as the project files (or in your compiler include path).

## Build & Run
**Option A: g++ / clang++ (Linux, macOS, MinGW)**

From the project folder:
```cpp
g++ -std=c++17 -O2 -o hotel_app ProjectCP2.cpp HotelSystem.cpp Scheduler.cpp
./hotel_app
```
**Option B: Visual Studio (Windows)**

1.Create a new Console C++ project

2.Add all .cpp and .h files

3.Ensure json.hpp is included in the project

4.Set C++ Language Standard to C++17

5.Build and run

## Usage

When you run the program, you’ll see a menu like:

-Add / list / edit / delete customers

-Add / list / cancel reservations

-List available rooms

-Run scheduler

-Admin menu

-Find customer by ID

-List all rooms

Exit (0) saves all data.

## Admin Credentials

Admin login is hardcoded:

**Username: admin**

**Password: 1234**

*(See HotelSystem::adminLogin())*

## Notes & Known Portability Detail

This project uses localtime_s (Windows-safe version).
If you compile on Linux/macOS and get errors, you may need to replace localtime_s with localtime_r (or adjust the implementation in getToday() / date helpers).

## Possible Improvements

-Hash admin password / store credentials securely

-More robust date handling (timezone-safe, library-based)

-Better room availability display by date range (not just “has any reservation linked”)

-Unit tests for date parsing, overlap logic, and scheduler assignment
