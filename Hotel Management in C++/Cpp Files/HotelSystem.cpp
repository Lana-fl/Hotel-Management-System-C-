#include "HotelSystem.h"
#include "Scheduler.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <ctime>
#include <iomanip>


struct Date {
    int y, m, d;
};

// Parse YYYY-MM-DD
bool parseDate(const string& s, Date& dt) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    try {
        dt.y = stoi(s.substr(0, 4));
        dt.m = stoi(s.substr(5, 2));
        dt.d = stoi(s.substr(8, 2));
    }
    catch (...) {
        return false;
    }
    return true;
}

// Check real calendar validity (including leap years)
bool isValidCalendarDate(const Date& dt) {
    if (dt.y < 1900 || dt.m < 1 || dt.m > 12 || dt.d < 1) return false;

    int mdays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

    bool leap = (dt.y % 4 == 0 && dt.y % 100 != 0) || (dt.y % 400 == 0);
    if (leap) mdays[1] = 29;

    return dt.d <= mdays[dt.m - 1];
}

// Convert to time_t
time_t toTimeT(const Date& dt) {
    tm t = {};
    t.tm_year = dt.y - 1900;
    t.tm_mon = dt.m - 1;
    t.tm_mday = dt.d;
    return mktime(&t);
}

// Check if the date is before today's date
bool isBeforeToday(const Date& dt) {
    time_t now = time(nullptr);
    tm local{};
    localtime_s(&local, &now);

    tm today = local;
    today.tm_hour = today.tm_min = today.tm_sec = 0;

    return toTimeT(dt) < mktime(&today);
}



using namespace std;
using json = nlohmann::json;

// ================================================================
//                  CONSTRUCTOR & INITIALIZATION
// ================================================================
HotelSystem::HotelSystem() {
    initializeRooms();   // Build hotel structure
    loadData();          // Load customers, reservations, rooms
}

// ================================================================
//                  INITIALIZE FIXED HOTEL ROOMS
// ================================================================
void HotelSystem::initializeRooms() {
    rooms.clear();

    // Floor 1: Single (101-112)
    for (int i = 101; i <= 112; i++) {
        rooms.emplace_back(i, 1, "Single", 150.0);
    }

    // Floor 2: Double (201-208)
    for (int i = 201; i <= 208; i++) {
        rooms.emplace_back(i, 2, "Double", 220.0);
    }

    // Floor 3: Suite (301-306)
    for (int i = 301; i <= 306; i++) {
        rooms.emplace_back(i, 3, "Suite", 450.0);
    }
}

// ================================================================
//                  AUTO SAVE (AFTER EVERY CHANGE)
// ================================================================
void HotelSystem::autoSave() const {
    saveData();
}

// ================================================================
//                  MAIN SAVE FUNCTION
// ================================================================
void HotelSystem::saveData() const {
    saveCustomersJSON();
    saveReservationsJSON();
    saveRoomsJSON();
}
// ================================================================
//                  MAIN LOAD FUNCTION
// ================================================================
void HotelSystem::loadData() {
    loadCustomersJSON();
    loadReservationsJSON();
    loadRoomsJSON();

    // Fix next IDs based on loaded data
    for (auto& c : customers)
        nextCustomerId = max(nextCustomerId, c.getId() + 1);

    for (auto& r : reservations)
        nextReservationId = max(nextReservationId, r.getId() + 1);
}

// ================================================================
//                  JSON LOAD FUNCTIONS
// ================================================================
void HotelSystem::loadCustomersJSON() {
    customers.clear();

    ifstream f(customersFile);
    if (!f.good()) return;

    json j;
    f >> j;

    for (auto& obj : j) {
        customers.push_back(Customer::from_json(obj));
    }
}

void HotelSystem::loadReservationsJSON() {
    reservations.clear();

    ifstream f(reservationsFile);
    if (!f.good()) return;

    json j;
    f >> j;

    for (auto& obj : j) {
        reservations.push_back(Reservation::from_json(obj));
    }
}

void HotelSystem::loadRoomsJSON() {
    ifstream f(roomsFile);
    if (!f.good()) return;  // If file missing, use initialized rooms

    json j; f >> j;

    // Overwrite initialized rooms (same room numbers)
    unordered_map<int, Room> temp;

    for (auto& obj : j) {
        Room r = Room::from_json(obj);
        temp[r.getRoomNumber()] = r;
    }

    // Update existing rooms with comments + history
    for (auto& r : rooms) {
        if (temp.count(r.getRoomNumber())) {
            Room loaded = temp[r.getRoomNumber()];

            // Update comments
            for (auto& c : loaded.getComments())
                r.addComment(c);

            // Update reservation IDs
            for (auto& rid : loaded.getReservationIds())
                r.addReservation(rid);
        }
    }
}

// ================================================================
//                  JSON SAVE FUNCTIONS
// ================================================================
void HotelSystem::saveCustomersJSON() const {
    json j = json::array();
    for (auto& c : customers)
        j.push_back(c.to_json());

    ofstream f(customersFile);
    f << j.dump(4);
}

void HotelSystem::saveReservationsJSON() const {
    json j = json::array();
    for (auto& r : reservations)
        j.push_back(r.to_json());

    ofstream f(reservationsFile);
    f << j.dump(4);
}

void HotelSystem::saveRoomsJSON() const {
    json j = json::array();
    for (auto& r : rooms)
        j.push_back(r.to_json());

    ofstream f(roomsFile);
    f << j.dump(4);
}

// ================================================================
//          HELPER FUNCTIONS (DATE, PRICES, FINDERS)
// ================================================================
double HotelSystem::getBasePriceForRoomType(const string& type) const {
    if (type == "Single") return 150.0;
    if (type == "Double") return 220.0;
    if (type == "Suite")  return 450.0;
    return 0;
}

bool HotelSystem::validDate(const string& d) const {
    return d.size() == 10 &&
        d[4] == '-' && d[7] == '-' &&
        isdigit(d[0]) && isdigit(d[1]) &&
        isdigit(d[2]) && isdigit(d[3]) &&
        isdigit(d[5]) && isdigit(d[6]) &&
        isdigit(d[8]) && isdigit(d[9]);
}

// Count how many nights (simple YYYY-MM-DD string compare)
int HotelSystem::calculateNights(const string& in, const string& out) const {
    int y1 = stoi(in.substr(0, 4)), m1 = stoi(in.substr(5, 2)), d1 = stoi(in.substr(8, 2));
    int y2 = stoi(out.substr(0, 4)), m2 = stoi(out.substr(5, 2)), d2 = stoi(out.substr(8, 2));

    tm a = {};
    a.tm_year = y1 - 1900; a.tm_mon = m1 - 1; a.tm_mday = d1;

    tm b = {};
    b.tm_year = y2 - 1900; b.tm_mon = m2 - 1; b.tm_mday = d2;

    time_t t1 = mktime(&a);
    time_t t2 = mktime(&b);

    return (t2 - t1) / (60 * 60 * 24);
}

// ================================================================
//               FINDERS FOR CUSTOMERS / ROOMS / RESERVATIONS
// ================================================================
Customer* HotelSystem::getCustomerRef(int id) {
    for (auto& c : customers)
        if (c.getId() == id) return &c;
    return nullptr;
}

optional<Customer> HotelSystem::getCustomerOptional(int id) const {
    for (auto& c : customers)
        if (c.getId() == id) return c;
    return nullopt;
}

Reservation* HotelSystem::getReservationRef(int id) {
    for (auto& r : reservations)
        if (r.getId() == id) return &r;
    return nullptr;
}

const Reservation* HotelSystem::getReservationConst(int id) const {
    for (auto& r : reservations)
        if (r.getId() == id) return &r;
    return nullptr;
}

Room* HotelSystem::getRoomRef(int roomNumber) {
    for (auto& r : rooms)
        if (r.getRoomNumber() == roomNumber) return &r;
    return nullptr;
}

const Room* HotelSystem::getRoomConst(int roomNumber) const {
    for (auto& r : rooms)
        if (r.getRoomNumber() == roomNumber) return &r;
    return nullptr;
}

// ================================================================
//                     CUSTOMER OPERATIONS
// ================================================================
#include <regex> // Make sure this is included at the top of HotelSystem.cpp

void HotelSystem::addCustomer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string name, phone, email;

    cout << "\nEnter Customer Name: ";
    getline(cin, name);

    // Phone validation: Lebanese local numbers xx xxx xxx
// Valid prefixes: 03, 70, 71, 76, 78, 81
    const regex phonePattern(R"(^((03|70|71|76|78|81) \d{3} \d{3})$)");
    do {
        cout << "Enter Phone Number (format xx xxx xxx, Lebanon local): ";
        getline(cin, phone);
        if (!regex_match(phone, phonePattern))
            cout << "Invalid phone format. Example: 70 123 456\n";
    } while (!regex_match(phone, phonePattern));

    // Email validation: standard email pattern, ends with valid domain
    const regex emailPattern(
        R"(^[A-Za-z0-9._%+-]+@((gmail\.com)|(hotmail\.com)|(lau\.edu)|([A-Za-z0-9.-]+\.com))$)"
    );


    do {
        cout << "Enter Email: ";
        getline(cin, email);
        if (!regex_match(email, emailPattern))
            cout << "Invalid email. Must contain @ and end with .com, gmail.com, hotmail.com, or .lau.edu\n";
    } while (!regex_match(email, emailPattern));

    customers.emplace_back(nextCustomerId++, name, phone, email);
    cout << "Customer added successfully.\n";

    autoSave();
}



void HotelSystem::editCustomer() {
    cout << "Enter Customer ID to Edit: ";
    int id; cin >> id;

    Customer* c = getCustomerRef(id);
    if (!c) {
        cout << "Customer not found.\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string name, phone, email;

    cout << "New Name (leave empty to keep same): ";
    getline(cin, name);

    cout << "New Phone (leave empty to keep same): ";
    getline(cin, phone);

    cout << "New Email (leave empty to keep same): ";
    getline(cin, email);

    if (!name.empty()) c->setName(name);
    if (!phone.empty()) c->setPhone(phone);
    if (!email.empty()) c->setEmail(email);

    cout << "Customer updated.\n";
    autoSave();
}

void HotelSystem::deleteCustomer() {
    cout << "Enter Customer ID to Delete: ";
    int id; cin >> id;

    // Cannot delete customer with existing reservations
    for (auto& r : reservations)
        if (r.getCustomerId() == id) {
            cout << "Cannot delete this customer. They have reservations.\n";
            return;
        }

    customers.erase(
        remove_if(customers.begin(), customers.end(),
            [&](const Customer& c) { return c.getId() == id; }),
        customers.end()
    );

    cout << "Customer deleted.\n";
    autoSave();
}

void HotelSystem::listCustomers() const {
    cout << "\n===== Customer List =====\n";

    if (customers.empty()) {
        cout << "No customers found.\n";
        return;
    }

    for (auto& c : customers) {
        cout << "ID: " << c.getId()
            << " | Name: " << c.getName()
            << " | Phone: " << c.getPhone()
            << " | Email: " << c.getEmail()
            << " | Past Bookings: " << c.getTotalBookings()
            << "\n";
    }
}

void HotelSystem::findCustomerByIdMenu() const {
    cout << "Enter Customer ID: ";
    int id; cin >> id;

    auto oc = getCustomerOptional(id);
    if (!oc) {
        cout << "Customer not found.\n";
        return;
    }

    auto c = oc.value();

    cout << "\n===== Customer Details =====\n";
    cout << "ID: " << c.getId() << "\n";
    cout << "Name: " << c.getName() << "\n";
    cout << "Phone: " << c.getPhone() << "\n";
    cout << "Email: " << c.getEmail() << "\n";
    cout << "Past Bookings: " << c.getTotalBookings() << "\n";

    cout << "\n===== Customer Reservations =====\n";

    for (auto& r : reservations) {
        if (r.getCustomerId() == id) {
            cout << "Reservation #" << r.getId()
                << " | Room " << r.getRoomNumber()
                << " | " << r.getCheckIn() << " -> " << r.getCheckOut()
                << " | Nights: " << r.getNights()
                << " | Final Price: $" << r.getFinalPrice()
                << "\n";
        }
    }
}
// ================================================================
//                          ROOMS DISPLAY
// ================================================================
void HotelSystem::listAllRooms() const {
    cout << "\n===== ALL ROOMS =====\n";

    for (auto& r : rooms) {
        cout << "Room " << r.getRoomNumber()
            << " | Floor: " << r.getFloor()
            << " | Type: " << r.getType()
            << " | Price: $" << r.getPrice()
            << " | Reservations Linked: " << r.getReservationIds().size()
            << "\n";
    }
}

void HotelSystem::listAvailableRooms() const {
    cout << "\n===== Available Rooms =====\n";

    for (auto& room : rooms) {
        bool isReserved = false;
        for (auto& res : reservations) {
            if (res.getRoomNumber() == room.getRoomNumber()) {
                isReserved = true;
                break;
            }
        }

        if (!isReserved) {
            cout << "Room " << room.getRoomNumber()
                << " | Floor " << room.getFloor()
                << " | Type: " << room.getType()
                << " | Price: $" << room.getPrice()
                << " (Available)\n";
        }
    }
}

// ================================================================
//                     ADD RESERVATION
// ================================================================
void HotelSystem::addReservation() {
    cout << "\n===== New Reservation =====\n";

    cout << "Enter Customer ID: ";
    int cid;
    cin >> cid;

    Customer* cu = getCustomerRef(cid);
    if (!cu) {
        cout << "Customer not found.\n";
        return;
    }

    string type;
    cout << "Room Type (Single / Double / Suite): ";
    cin >> type;

    if (type != "Single" && type != "Double" && type != "Suite") {
        cout << "Invalid room type.\n";
        return;
    }

    // ============================
    // DATE VALIDATION WITH RE-ASK
    // ============================

    string checkInStr, checkOutStr;
    Date checkIn, checkOut;

    // Ask CHECK-IN date until valid
    while (true) {
        cout << "Check-in date (YYYY-MM-DD): ";
        cin >> checkInStr;

        if (!parseDate(checkInStr, checkIn) ||
            !isValidCalendarDate(checkIn) ||
            isBeforeToday(checkIn))
        {
            cout << "Invalid check-in date. Please try again.\n";
            continue;
        }
        break; // valid
    }

    // Ask CHECK-OUT date until valid
    while (true) {
        cout << "Check-out date (YYYY-MM-DD): ";
        cin >> checkOutStr;

        if (!parseDate(checkOutStr, checkOut) ||
            !isValidCalendarDate(checkOut) ||
            isBeforeToday(checkOut))
        {
            cout << "Invalid check-out date. Please try again.\n";
            continue;
        }

        if (toTimeT(checkIn) >= toTimeT(checkOut)) {
            cout << "Check-out date must be after check-in date. Try again.\n";
            continue;
        }

        break; // valid
    }

    // ============================
    // ROOM ASSIGNMENT
    // ============================

    int assignedRoom = -1;
    int assignedFloor = 0;

    for (auto& room : rooms) {
        if (room.getType() == type) {
            bool occupied = false;

            for (auto& res : reservations) {
                if (res.getRoomNumber() == room.getRoomNumber()) {
                    // checking overlap
                    if (!(checkOutStr <= res.getCheckIn() ||
                        res.getCheckOut() <= checkInStr))
                    {
                        occupied = true;
                        break;
                    }
                }
            }

            if (!occupied) {
                assignedRoom = room.getRoomNumber();
                assignedFloor = room.getFloor();
                break;
            }
        }
    }

    if (assignedRoom == -1) {
        cout << "No available rooms of this type.\n";
        return;
    }

    // ============================
    // PRICE AND RESERVATION SAVE
    // ============================

    int nights = calculateNights(checkInStr, checkOutStr);
    double basePrice = getBasePriceForRoomType(type);
    double finalPrice = nights * basePrice;

    if (cu->getTotalBookings() >= 3)
        finalPrice *= 0.90; // discount

    Reservation r(
        nextReservationId++,
        cid,
        checkInStr,
        checkOutStr,
        assignedRoom,
        assignedFloor,
        type,
        nights,
        finalPrice
    );

    reservations.push_back(r);

    Room* roomRef = getRoomRef(assignedRoom);
    if (roomRef)
        roomRef->addReservation(r.getId());

    cu->incrementBookings();

    printInvoice(r, cu->getTotalBookings() >= 3);

    cout << "Reservation added.\n";
    autoSave();
}


void HotelSystem::cancelReservation() {
    cout << "Enter Reservation ID to cancel: ";
    int rid; cin >> rid;

    Reservation* r = getReservationRef(rid);
    if (!r) {
        cout << "Reservation not found.\n";
        return;
    }

    int roomNum = r->getRoomNumber();

    // Remove from room history
    Room* room = getRoomRef(roomNum);
    if (room) {
        auto& vec = const_cast<vector<int>&>(room->getReservationIds());
        vec.erase(remove(vec.begin(), vec.end(), rid), vec.end());
    }

    // Remove reservation entirely
    reservations.erase(
        remove_if(reservations.begin(), reservations.end(),
            [&](const Reservation& rr) { return rr.getId() == rid; }),
        reservations.end()
    );

    cout << "Reservation cancelled.\n";
    autoSave();
}


// ================================================================
//                        PRINT INVOICE
// ================================================================
void HotelSystem::printInvoice(const Reservation& r, bool loyalty) {
    cout << "\n========= INVOICE =========\n";
    cout << "Reservation ID: " << r.getId() << "\n";
    cout << "Customer ID:    " << r.getCustomerId() << "\n";
    cout << "Room:           " << r.getRoomNumber() << " (" << r.getRoomType() << ")\n";
    cout << "Check In:       " << r.getCheckIn() << "\n";
    cout << "Check Out:      " << r.getCheckOut() << "\n";
    cout << "Nights:         " << r.getNights() << "\n";
    cout << "Base Price:     $" << getBasePriceForRoomType(r.getRoomType()) << " per night\n";

    if (loyalty)
        cout << "Loyalty Discount: 10% applied\n";

    cout << "FINAL PRICE:    $" << r.getFinalPrice() << "\n";
    cout << "============================\n";
    cout << endl;
}

// ================================================================
//                     ADMIN LOGIN + ADMIN MENU
// ================================================================
bool HotelSystem::adminLogin() {
    cout << "\n===== ADMIN LOGIN =====\n";

    string user, pass;
    cout << "Username: ";
    cin >> user;
    cout << "Password: ";
    cin >> pass;

    if (user == "admin" && pass == "1234") {
        cout << "Admin login successful.\n";
        return true;
    }

    cout << "Invalid credentials.\n";
    return false;
}

void HotelSystem::adminMenu() {
    if (!adminLogin()) return;

    int choice = -1;
    while (choice != 0) {
        cout << "\n===== ADMIN MENU =====\n";
        cout << "1) Cancel Reservation\n";
        cout << "2) Modify Reservation Price\n";
        cout << "3) Add Comment to Room\n";
        cout << "4) View All Room Comments\n";
        cout << "5) List All Reservations\n";
        cout << "6) List Past Reservations\n";
        cout << "7) List Future Reservations\n";
        cout << "0) Exit Admin Menu\n";
        cout << "Choice: ";
        cin >> choice;

        switch (choice) {
        case 1: adminCancelReservation(); break;
        case 2: adminModifyReservationPrice(); break;
        case 3: adminAddRoomComment(); break;
        case 4: adminViewAllComments(); break;
        case 5: listReservations(); break;
        case 6: listPastReservations(); break;
        case 7: listFutureReservations(); break;
        case 0: break;
        default: cout << "Invalid choice.\n";
        }
    }
}
// ================================================================
//                    ADMIN: CANCEL RESERVATION
// ================================================================
void HotelSystem::adminCancelReservation() {
    cout << "\nEnter Reservation ID to cancel: ";
    int rid; cin >> rid;

    Reservation* r = getReservationRef(rid);
    if (!r) {
        cout << "Reservation not found.\n";
        return;
    }

    int roomNum = r->getRoomNumber();

    // Remove from room history
    Room* room = getRoomRef(roomNum);
    if (room) {
        auto& vec = const_cast<vector<int>&>(room->getReservationIds());
        vec.erase(remove(vec.begin(), vec.end(), rid), vec.end());
    }
    reservations.erase(
        remove_if(reservations.begin(), reservations.end(),
            [&](const Reservation& rr) { return rr.getId() == rid; }),
        reservations.end()
    );

    cout << "Reservation cancelled.\n";
    autoSave();
}

// ================================================================
//                 ADMIN: MODIFY RESERVATION PRICE
// ================================================================
void HotelSystem::adminModifyReservationPrice() {
    cout << "\nEnter Reservation ID: ";
    int rid; cin >> rid;

    Reservation* r = getReservationRef(rid);
    if (!r) {
        cout << "Reservation not found.\n";
        return;
    }

    cout << "Current price: $" << r->getFinalPrice() << "\n";
    cout << "Enter new price: ";
    double p; cin >> p;

    r->setFinalPrice(p);

    cout << "Reservation price updated.\n";
    autoSave();
}

// ================================================================
//                 ADMIN: ADD COMMENT TO ROOM
// ================================================================
void HotelSystem::adminAddRoomComment() {
    cout << "\nEnter Room Number: ";
    int rn; cin >> rn;

    Room* room = getRoomRef(rn);
    if (!room) {
        cout << "Room not found.\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter comment: ";
    string c;
    getline(cin, c);

    room->addComment(c);

    cout << "Comment added.\n";
    autoSave();
}

// ================================================================
//                 ADMIN: VIEW ALL COMMENTS
// ================================================================
void HotelSystem::adminViewAllComments() {
    cout << "\n===== ROOM COMMENTS =====\n";

    for (auto& r : rooms) {
        cout << "\nRoom " << r.getRoomNumber()
            << " (Floor " << r.getFloor()
            << ", " << r.getType() << "):\n";

        if (r.getComments().empty()) {
            cout << "  No comments.\n";
            continue;
        }

        for (auto& c : r.getComments()) {
            cout << "  - " << c << "\n";
        }
    }
}

// ================================================================
//                 LIST RESERVATIONS (SORTED)
// ================================================================
void HotelSystem::sortReservations() {
    sort(reservations.begin(), reservations.end(),
        [&](const Reservation& a, const Reservation& b) {
            if (a.getCustomerId() != b.getCustomerId())
                return a.getCustomerId() < b.getCustomerId();
            return a.getCheckIn() < b.getCheckIn();
        });
}

void HotelSystem::listReservations() const {
    cout << "\n===== ALL RESERVATIONS =====\n";

    if (reservations.empty()) {
        cout << "No reservations found.\n";
        return;
    }

    for (auto& r : reservations) {
        cout << "Res#" << r.getId()
            << " | Cust#" << r.getCustomerId()
            << " | Room " << r.getRoomNumber()
            << " | " << r.getCheckIn() << " -> " << r.getCheckOut()
            << " | Nights: " << r.getNights()
            << " | Price: $" << r.getFinalPrice()
            << "\n";
    }
}

void HotelSystem::listPastReservations() const {
    cout << "\n===== PAST RESERVATIONS =====\n";

    string today = getToday(); 

    for (auto& r : reservations) {
        if (r.getCheckOut() < today) {
            cout << "Res#" << r.getId()
                << " | Cust#" << r.getCustomerId()
                << " | Room " << r.getRoomNumber()
                << " | " << r.getCheckIn() << " -> " << r.getCheckOut()
                << "\n";
        }
    }
}

void HotelSystem::listFutureReservations() const {
    cout << "\n===== FUTURE RESERVATIONS =====\n";

    string today = getToday();

    for (auto& r : reservations) {
        if (r.getCheckIn() > today) {
            cout << "Res#" << r.getId()
                << " | Cust#" << r.getCustomerId()
                << " | Room " << r.getRoomNumber()
                << " | " << r.getCheckIn() << " -> " << r.getCheckOut()
                << "\n";
        }
    }
}


string HotelSystem::getToday() const {
    time_t t = time(nullptr);
    struct tm now;
    localtime_s(&now, &t);

    stringstream ss;
    ss << put_time(&now, "%Y-%m-%d");
    cout << "Today is " << ss.str()<<endl;
    return ss.str();
}

// ================================================================
//                        RUN SCHEDULER
// ================================================================
void HotelSystem::runScheduler() {
    Scheduler::allocateRooms(reservations, rooms);
    cout << "Scheduling completed.\n";
    autoSave();
}

// ================================================================
//                       MAIN MENU HANDLER
// ================================================================
void HotelSystem::mainMenuChoice(int choice) {
    switch (choice) {
    case 1: addCustomer(); break;
    case 2: listCustomers(); break;
    case 3: editCustomer(); break;
    case 4: deleteCustomer(); break;
    case 5: addReservation(); break;
    case 6: listReservations(); break;
    case 7: cancelReservation(); break;
    case 8: listAvailableRooms(); break;
    case 9: runScheduler(); break;
    case 10: adminMenu(); break;
    case 11: findCustomerByIdMenu(); break;
    case 12: listAllRooms(); break;
    default: cout << "Invalid choice.\n";
    }
}

