#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include "Reservation.h"
#include "Room.h"

using namespace std;

class Scheduler {
public:
    // Assigns reservations to existing hotel rooms (no room creation)
    static void allocateRooms(vector<Reservation>& reservations, vector<Room>& rooms);
};

#endif