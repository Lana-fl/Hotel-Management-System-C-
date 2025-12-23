#ifndef ROOM_H
#define ROOM_H

#include <vector>
#include <string>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class Room {
private:
    int roomNumber;                    // like 101, 205, 303
    int floor;                         // 1, 2, or 3
    string type;                       // "Single", "Double", "Suite"
    double price;                      // price per night

    vector<int> reservationIds;        // Reservations linked to this room
    vector<string> comments;           // Admin comments for this room

public:
    Room() : roomNumber(-1), floor(0), price(0.0) {}

    Room(int roomNumber, int floor, const string& type, double price)
        : roomNumber(roomNumber), floor(floor), type(type), price(price) {
    }

    // Getters
    int getRoomNumber() const { return roomNumber; }
    int getFloor() const { return floor; }
    const string& getType() const { return type; }
    double getPrice() const { return price; }
    const vector<int>& getReservationIds() const { return reservationIds; }
    const vector<string>& getComments() const { return comments; }

    // Mutators
    void addReservation(int reservationId) {
        reservationIds.push_back(reservationId);
    }

    void clearReservations() {
        reservationIds.clear();
    }

    void addComment(const string& comment) {
        comments.push_back(comment);
    }

    void setPrice(double newPrice) {
        price = newPrice;
    }

    // JSON Serialization (Option 1)
    json to_json() const {
        return json{
            {"roomNumber", roomNumber},
            {"floor", floor},
            {"type", type},
            {"price", price},
            {"comments", comments},
            {"reservationIds", reservationIds}
        };
    }

    static Room from_json(const json& j) {
        Room r(
            j.at("roomNumber").get<int>(),
            j.at("floor").get<int>(),
            j.at("type").get<string>(),
            j.at("price").get<double>()
        );

        if (j.contains("comments"))
            r.comments = j.at("comments").get<vector<string>>();

        if (j.contains("reservationIds"))
            r.reservationIds = j.at("reservationIds").get<vector<int>>();

        return r;
    }
};

#endif