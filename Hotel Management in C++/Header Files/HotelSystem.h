#ifndef HOTELSYSTEM_H
#define HOTELSYSTEM_H

#include <vector>
#include <string>
#include <optional>
#include <unordered_map>

#include "Customer.h"
#include "Reservation.h"
#include "Room.h"

using namespace std;

class HotelSystem {

private:
    vector<Customer> customers;
    vector<Reservation> reservations;
    vector<Room> rooms;

    int nextCustomerId = 1;
    int nextReservationId = 1;

    const string customersFile = "customers.json";
    const string reservationsFile = "reservations.json";
    const string roomsFile = "rooms.json";

public:
    HotelSystem();
    void loadData();
    void saveData() const;
    string getToday() const;

    // ==========================================================
    // MENUS
    // ==========================================================
    void mainMenuChoice(int choice);
    void adminMenu();

    // ==========================================================
    // CUSTOMER OPS
    // ==========================================================
    void addCustomer();
    void editCustomer();
    void deleteCustomer();
    void listCustomers() const;
    void findCustomerByIdMenu() const;

    // ==========================================================
    // RESERVATIONS
    // ==========================================================
    void addReservation();
    void cancelReservation();
    void listReservations() const;
    void listPastReservations() const;
    void listFutureReservations() const;

    // ==========================================================
    // ROOMS
    // ==========================================================
    void initializeRooms();
    void listAvailableRooms() const;
    void listAllRooms() const;

    // ==========================================================
    // INVOICE
    // ==========================================================
    void printInvoice(const Reservation& r, bool applyLoyalty);

    // ==========================================================
    // ADMIN
    // ==========================================================
    bool adminLogin();
    void adminModifyReservationPrice();
    void adminAddRoomComment();
    void adminViewAllComments();
    void adminCancelReservation();

    // ==========================================================
    // SCHEDULER
    // ==========================================================
    void runScheduler();
    

private:

    Customer* getCustomerRef(int id);
    optional<Customer> getCustomerOptional(int id) const;
    Reservation* getReservationRef(int id);
    const Reservation* getReservationConst(int id) const;
    Room* getRoomRef(int roomNumber);
    const Room* getRoomConst(int roomNumber) const;

    int calculateNights(const string& in, const string& out) const;
    bool validDate(const string& d) const;
    double getBasePriceForRoomType(const string& type) const;

    void sortReservations();
    void autoSave() const;

    // ==========================================================
    // JSON
    // ==========================================================
    void loadCustomersJSON();
    void loadReservationsJSON();
    void loadRoomsJSON();
    void saveCustomersJSON() const;
    void saveReservationsJSON() const;
    void saveRoomsJSON() const;

    // ==========================================================
    // REQUIRED FIX — UNIVERSAL "00" EXIT HELPER
    // ==========================================================
    bool isCancel(const string& s) const;   // <--- ADDED THIS

};


#endif
