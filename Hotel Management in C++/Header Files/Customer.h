#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <string>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class Customer {
private:
    int id;
    string name;
    string phone;
    string email;
    int totalBookings;   // NEW: number of past completed reservations

public:
    Customer() : id(-1), totalBookings(0) {}

    Customer(int id, const string& name, const string& phone, const string& email, int totalBookings = 0)
        : id(id), name(name), phone(phone), email(email), totalBookings(totalBookings) {
    }

    // Getters
    int getId() const { return id; }
    const string& getName() const { return name; }
    const string& getPhone() const { return phone; }
    const string& getEmail() const { return email; }
    int getTotalBookings() const { return totalBookings; }

    // Setters
    void setName(const string& v) { name = v; }
    void setPhone(const string& v) { phone = v; }
    void setEmail(const string& v) { email = v; }
    void incrementBookings() { totalBookings++; }

    // JSON Serialization
    json to_json() const {
        return json{
            {"id", id},
            {"name", name},
            {"phone", phone},
            {"email", email},
            {"totalBookings", totalBookings}
        };
    }

    static Customer from_json(const json& j) {
        return Customer(
            j.at("id").get<int>(),
            j.at("name").get<string>(),
            j.at("phone").get<string>(),
            j.at("email").get<string>(),
            j.value("totalBookings", 0)
        );
    }
};

#endif