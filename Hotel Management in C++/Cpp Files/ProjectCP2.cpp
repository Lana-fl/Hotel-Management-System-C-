#include <iostream>
#include "HotelSystem.h"

using namespace std;

int main() {
    HotelSystem system;

    int choice = -1;

    while (choice != 0) {
        cout << "\n========== SMART HOTEL SYSTEM ==========\n";
        cout << "1)  Add Customer\n";
        cout << "2)  List Customers\n";
        cout << "3)  Edit Customer\n";
        cout << "4)  Delete Customer\n";
        cout << "5)  Add Reservation\n";
        cout << "6)  List Reservations\n";
        cout << "7)  Cancel Reservation (User)\n";
        cout << "8)  List Available Rooms\n";
        cout << "9)  Run Scheduler\n";
        cout << "10) Admin Menu\n";
        cout << "11) Find Customer by ID\n";
        cout << "12) List All Rooms\n";
        cout << "0)  Exit\n";
        cout << "Enter choice: ";

        cin >> choice;

        if (!cin) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
            continue;
        }

        if (choice == 0) break;

        system.mainMenuChoice(choice);
    }

    cout << "Exiting... Saving data.\n";
    system.saveData();
    return 0;
}