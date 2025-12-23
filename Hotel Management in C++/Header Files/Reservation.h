#ifndef RESERVATION_H
#define RESERVATION_H

#include <string>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

class Reservation {
private:
    int id;
    int customerId;
    string checkIn;
    string checkOut;
    int roomNumber;
    int floor;
    string roomType;
    int nights;
    double finalPrice;

public:
    Reservation()
        : id(-1), customerId(-1), roomNumber(-1),
        floor(0), nights(0), finalPrice(0.0) {
    }

    Reservation(int id, int customerId, const string& in, const string& out,
        int roomNumber, int floor, const string& type,
        int nights, double finalPrice)
        : id(id), customerId(customerId),
        checkIn(in), checkOut(out),
        roomNumber(roomNumber), floor(floor),
        roomType(type), nights(nights), finalPrice(finalPrice) {
    }

    
    int getId() const { return id; }
    int getCustomerId() const { return customerId; }
    const string& getCheckIn() const { return checkIn; }
    const string& getCheckOut() const { return checkOut; }
    int getRoomNumber() const { return roomNumber; }
    int getFloor() const { return floor; }
    const string& getRoomType() const { return roomType; }
    int getNights() const { return nights; }
    double getFinalPrice() const { return finalPrice; }

    void setRoomNumber(int rn) { roomNumber = rn; }

    void setFinalPrice(double p) { finalPrice = p; }

    bool overlaps(const Reservation& other) const {
        return !(checkOut <= other.getCheckIn() || other.getCheckOut() <= checkIn);
    }

    json to_json() const {
        return json{
            {"id", id},
            {"customerId", customerId},
            {"checkIn", checkIn},
            {"checkOut", checkOut},
            {"roomNumber", roomNumber},
            {"floor", floor},
            {"roomType", roomType},
            {"nights", nights},
            {"finalPrice", finalPrice}
        };
    }

    static Reservation from_json(const json& j) {
        return Reservation(
            j.at("id"),
            j.at("customerId"),
            j.at("checkIn"),
            j.at("checkOut"),
            j.at("roomNumber"),
            j.at("floor"),
            j.at("roomType"),
            j.value("nights", 0),
            j.value("finalPrice", 0.0)
        );
    }
};

#endif
