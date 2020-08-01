#include <deque>
#include <iostream>
#include <unordered_map>

using namespace std;

class BookingManager {
public:
    void Book(long time, const string &hotel_name, int client_id, int room_count) {
        bookings.push_back({time, hotel_name, client_id, room_count});
        rooms_booked[hotel_name] += room_count;
        client_last_booking[hotel_name][client_id] = time;

        PopUnnecessary(time);
    }

    int Clients(const string &hotel_name) const {
        return client_last_booking.count(hotel_name) > 0 ? static_cast<int>(client_last_booking.at(hotel_name).size()) : 0;
    }

    int Rooms(const string &hotel_name) const {
        return rooms_booked.count(hotel_name) > 0 ? rooms_booked.at(hotel_name) : 0;
    }

private:
    void PopUnnecessary(long time) {
        while (bookings.front().time <= (time - DAY_IN_SECONDS)) {
            rooms_booked[bookings.front().hotel_name] -= bookings.front().room_count;
            if (auto client_it = client_last_booking[bookings.front().hotel_name].find(bookings.front().client_id); client_it->second == bookings.front().time) {
                client_last_booking[bookings.front().hotel_name].erase(client_it);
            }
            bookings.pop_front();
        }
    }

    struct Booking {
        long time;
        string hotel_name;
        int client_id, room_count;
    };

    static const long DAY_IN_SECONDS = 60 * 60 * 24;

    deque<Booking> bookings;
    unordered_map<string, int> rooms_booked;
    unordered_map<string, unordered_map<int, long>> client_last_booking;
};

int main() {
    BookingManager manager;

    int N;
    cin >> N;
    for (int i = 0; i < N; ++i) {
        string query_type;
        cin >> query_type;
        if (query_type == "BOOK") {
            long time;
            string hotel_name;
            int client_id, room_count;
            cin >> time >> hotel_name >> client_id >> room_count;

            manager.Book(time, hotel_name, client_id, room_count);
        } else if (query_type == "CLIENTS") {
            string hotel_name;
            cin >> hotel_name;

            cout << manager.Clients(hotel_name) << endl;
        } else if (query_type == "ROOMS") {
            string hotel_name;
            cin >> hotel_name;

            cout << manager.Rooms(hotel_name) << endl;
        }
    }


    return 0;
}
