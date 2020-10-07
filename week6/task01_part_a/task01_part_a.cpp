#include <cmath>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

const double PI = 3.1415926535;

struct Coords {
    Coords(double latitude_degrees, double longitude_degrees) {
        latitude = latitude_degrees / 180 * PI;
        longitude = longitude_degrees / 180 * PI;
    }

    double latitude = 0;
    double longitude = 0;

    double operator-(const Coords &other) const {
        return acos(
                sin(this->latitude) * sin(other.latitude) +
                cos(this->latitude) * cos(other.latitude) *
                cos(abs(this->longitude - other.longitude))
        ) * 6371000;
    }
};

struct AddStopRequest {
    string stop_name;
    Coords coords;
};

struct AddBusRequest {
    string bus_name;
    vector<string> stops;
};

struct DbInputRequests {
    vector<unique_ptr<AddStopRequest>> add_stop_requests;
    vector<unique_ptr<AddBusRequest>> add_bus_requests;
};


class Database {
public:
    struct BusInfo {
        size_t num_stops, num_unique_stops;
        double bus_length;
    };

    void AddStop(string name, Coords coords) {
        stops.insert({move(name), coords});
    }

    void AddBus(string name, vector<string> stops) {
        buses[move(name)] = {{stops.size(), CalculateUniqueStops(stops), -1}, move(stops)};
    }


    void ApplyFillRequests(DbInputRequests requests) {
        for (unique_ptr<AddStopRequest> &stop_req : requests.add_stop_requests) {
            AddStop(move(stop_req->stop_name), move(stop_req->coords));
        }
        for (unique_ptr<AddBusRequest> &bus_req : requests.add_bus_requests) {
            AddBus(move(bus_req->bus_name), move(bus_req->stops));
        }
    }

    void CalculateBusLengthes() {
        for (auto&[name, bus] : buses) {
            if (bus.stops.size() <= 1) {
                continue;
            }

            double res_length = 0;
            auto prev_it = begin(bus.stops), next_it = next(prev_it);
            while (next_it != bus.stops.end()) {
                res_length += stops.at(*prev_it) - stops.at(*next_it);
                prev_it++, next_it++;
            }

            bus.busInfo.bus_length = res_length;
        }
    }

    const BusInfo* GetBusInfo(const string& bus_name) const {
        auto it = buses.find(bus_name);
        if (it == buses.end()) {
            return nullptr;
        } else {
            return &it->second.busInfo;
        }
    }


private:
    static size_t CalculateUniqueStops(const vector<string> &stops) {
        set<string> stops_set(begin(stops), end(stops));
        return stops_set.size();
    }

    struct Bus {
        BusInfo busInfo;
        vector<string> stops;
    };

    unordered_map<string, Coords> stops;
    unordered_map<string, Bus> buses;

};


unique_ptr<AddStopRequest> ParseStopInputRequest(string line) {
    stringstream ss(line);
    string stop_name;
    double latitude, longitude;

    ss.ignore(5);  // "Stop " length
    getline(ss, stop_name, ':');
    ss >> latitude;
    ss.ignore();  // ", "
    ss >> longitude;

    return make_unique<AddStopRequest>(AddStopRequest{stop_name, {latitude, longitude}});
}

unique_ptr<AddBusRequest> ParseBusInputRequest(string line) {
    string bus_name;
    vector<string> stops;

    size_t colon_pos = line.find(':', 4 /* "Bus " length */);
    bus_name = line.substr(4 /* "Bus " length */, (colon_pos - 4));

    size_t first_delim_pos = line.find_first_of(">-", colon_pos);
    bool is_circular = line[first_delim_pos] == '>';
    string delim = is_circular ? " > " : " - ";
    size_t delim_pos, prev_delim_pos = colon_pos + 2;  // ": " length
    while ((delim_pos = line.find(delim, prev_delim_pos)) != string::npos) {
        stops.push_back(line.substr(prev_delim_pos, (delim_pos - prev_delim_pos)));
        prev_delim_pos = delim_pos + 3;
    }
    stops.push_back(line.substr(prev_delim_pos, string::npos));

    if (!is_circular) {  // зациклить
        for (int i = static_cast<int>(stops.size()) - 2; i >= 0; --i) {
            stops.push_back(stops[i]);
        }
    }

    return make_unique<AddBusRequest>(AddBusRequest{move(bus_name), move(stops)});
}


DbInputRequests ParseDbInput() {
    DbInputRequests res;

    string line;
    getline(cin, line);
    int N = stoi(line);

    for (int n = 0; n < N; ++n) {
        getline(cin, line);

        if (string_view(line).substr(0, 4) == "Stop") {
            res.add_stop_requests.push_back(ParseStopInputRequest(move(line)));
        } else if (string_view(line).substr(0, 3) == "Bus") {
            res.add_bus_requests.push_back(ParseBusInputRequest(move(line)));
        }
    }

    return res;
}

vector<string> ParseBusRequests() {
    vector<string> res;

    string line;
    getline(cin, line);
    int N = stoi(line);

    for (int n = 0; n < N; ++n) {
        string query;
        cin >> query;

        if (query == "Bus") {
            cin.ignore();  // пробел после Bus
            string bus_name;
            getline(cin, bus_name);

            res.push_back(move(bus_name));

        }
    }

    return res;
}

int main() {
    Database db;

    DbInputRequests db_input_requests = ParseDbInput();

    db.ApplyFillRequests(move(db_input_requests));


    db.CalculateBusLengthes();

    // ==============================
    vector<string> bus_requests = ParseBusRequests();

    for (const string& bus_name : bus_requests) {
        const Database::BusInfo* bus_info = db.GetBusInfo(bus_name);
        if (!bus_info) {
            cout << "Bus " << bus_name << ": not found" << endl;
        } else {
            cout << "Bus " << bus_name << ": " << bus_info->num_stops << " stops on route, " << bus_info->num_unique_stops << " unique stops, " << bus_info->bus_length << " route length" << endl;
        }
    }
}
