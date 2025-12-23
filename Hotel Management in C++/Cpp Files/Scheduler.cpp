#include "Scheduler.h"
#include <algorithm>

using namespace std;

// Helper: Check if this room can take the reservation (no overlaps)
static bool roomIsFreeFor(const Room& room, const Reservation& res,
    const vector<Reservation>& reservations) {

    for (int rId : room.getReservationIds()) {
        for (const Reservation& existing : reservations) {
            if (existing.getId() == rId) {
                if (existing.overlaps(res)) {
                    return false; // conflict
                }
            }
        }
    }
    return true;
}

void Scheduler::allocateRooms(vector<Reservation>& reservations, vector<Room>& rooms) {
    // Sort reservations by check-in date
    sort(reservations.begin(), reservations.end(),
        [](const Reservation& a, const Reservation& b) {
            if (a.getCustomerId() == b.getCustomerId())
                return a.getCheckIn() < b.getCheckIn();
            return a.getCustomerId() < b.getCustomerId();
        });

    // Clear previous room assignment lists
    for (auto& r : rooms) {
        r.clearReservations();
    }

    // Try to assign each reservation to a room
    for (auto& res : reservations) {

        bool placed = false;

        for (auto& room : rooms) {

            // Room type must match reservation
            if (room.getType() != res.getRoomType())
                continue;

            // Check for overlap
            if (roomIsFreeFor(room, res, reservations)) {
                res.setRoomNumber(room.getRoomNumber());
                room.addReservation(res.getId());
                placed = true;
                break;
            }
        }

        if (!placed) {
            // Should never happen unless overbooked
            res.setRoomNumber(-1);
        }
    }
}