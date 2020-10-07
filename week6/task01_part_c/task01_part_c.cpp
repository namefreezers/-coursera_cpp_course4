#include <cmath>
#include <iostream>
#include <iomanip>
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
    unordered_map<string, double> distances;
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
    struct Stop {
        Coords coords;
        set<string> stop_in_buses;
        unordered_map<string, double> distances;

        double GetDistanceTo(const string& this_stop_name, const string& to_stop_name, const Stop& to_stop) {
            auto it = distances.find(to_stop_name);
            if (it != distances.end()) {
                return it->second;
            } else {
                return to_stop.distances.at(this_stop_name);
            }
        }
    };

    struct BusInfo {
        size_t num_stops, num_unique_stops;
        double bus_calculated_length;
        double bus_real_length;
    };

    struct Bus {
        BusInfo busInfo;
        vector<string> stops;
    };

    void AddStop(string name, Coords coords, unordered_map<string, double> distances) {
        stops.insert({move(name), {coords, {}, move(distances)}});
    }

    void AddBus(string bus_name, vector<string> stops_to_add) {
        auto it = buses.insert({move(bus_name), {{stops_to_add.size(), CalculateUniqueStops(stops_to_add), -1}, move(stops_to_add)}});

        const vector<string>& stops_this_bus = it.first->second.stops;
        const string this_bus_name = it.first->first;
        for (const string& stop : stops_this_bus) {
            stops.at(stop).stop_in_buses.insert(this_bus_name);
        }
    }


    void ApplyFillRequests(DbInputRequests requests) {
        for (unique_ptr<AddStopRequest> &stop_req : requests.add_stop_requests) {
            AddStop(move(stop_req->stop_name), move(stop_req->coords), move(stop_req->distances));
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

            double calculated_res_length = 0, real_res_length = 0;
            auto prev_it = begin(bus.stops), next_it = next(prev_it);
            while (next_it != bus.stops.end()) {
                // calculated
                calculated_res_length += stops.at(*prev_it).coords - stops.at(*next_it).coords;

                // real
                real_res_length += stops.at(*prev_it).GetDistanceTo(*prev_it, *next_it, stops.at(*next_it));

                prev_it++, next_it++;
            }

            bus.busInfo.bus_calculated_length = calculated_res_length;
            bus.busInfo.bus_real_length = real_res_length;
        }
    }

    const BusInfo *GetBusInfo(const string &bus_name) const {
        auto it = buses.find(bus_name);
        if (it == buses.end()) {
            return nullptr;
        } else {
            return &it->second.busInfo;
        }
    }



    const Stop* GetStopInfo(const string &stop_name) const {
        auto it = stops.find(stop_name);
        if (it == stops.end()) {
            return nullptr;
        } else {
            return &it->second;
        }
    }



private:
    static size_t CalculateUniqueStops(const vector<string> &stops) {
        set<string> stops_set(begin(stops), end(stops));
        return stops_set.size();
    }

    unordered_map<string, Stop> stops;
    unordered_map<string, Bus> buses;

};



class Database;

struct ReadRequest {
    virtual string ServeRequest(const Database&) = 0;
};

struct GetBusRequest : public ReadRequest {
    GetBusRequest(string bus_name_) : bus_name(move(bus_name_)) {}

    string bus_name;

    string ServeRequest(const Database& db) {
        stringstream ss;
        const Database::BusInfo *bus_info = db.GetBusInfo(bus_name);
        if (!bus_info) {
            ss << "Bus " << bus_name << ": not found";
        } else {
            ss << "Bus " << bus_name << ": " << bus_info->num_stops << " stops on route, " << bus_info->num_unique_stops << " unique stops, " <<  setprecision(6) << bus_info->bus_real_length << " route length, " <<  setprecision(6) << (bus_info->bus_real_length / bus_info->bus_calculated_length) << " curvature";
        }
        return ss.str();
    }
};


struct GetStopRequest : public ReadRequest {
    GetStopRequest(string stop_name_) : stop_name(move(stop_name_)) {}

    string stop_name;

    string ServeRequest(const Database& db) {
        stringstream ss;
        const Database::Stop* stop = db.GetStopInfo(stop_name);
        if (!stop) {
            ss << "Stop " << stop_name << ": not found";
        } else if (stop->stop_in_buses.empty()) {
            ss << "Stop " << stop_name << ": no buses";
        } else {
            ss << "Stop " << stop_name << ": buses";
            for (const string& bus_on_stop : stop->stop_in_buses) {
                ss << " " << bus_on_stop;
            }
        }
        return ss.str();
    }
};




unique_ptr<AddStopRequest> ParseStopInputRequest(string line) {
    stringstream ss(line);
    string stop_name;
    double latitude, longitude;
    unordered_map<string, double> distances;

    ss.ignore(5);  // "Stop " length
    getline(ss, stop_name, ':');

    ss >> latitude;
    ss.ignore();  // ", "
    ss >> longitude;
    ss.ignore();  // ", "

    double temp_dist;
    string temp_stop_dist_to;
    while (ss >> temp_dist) {
        ss.ignore(5);  // "m to " length
        getline(ss, temp_stop_dist_to, ',');
        distances.insert({move(temp_stop_dist_to), temp_dist});
    }

    return make_unique<AddStopRequest>(AddStopRequest{stop_name, {latitude, longitude}, move(distances)});
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

vector<unique_ptr<ReadRequest>> ParseReadRequests() {
    vector<unique_ptr<ReadRequest>> res;

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

            res.push_back(make_unique<GetBusRequest>(move(bus_name)));
        } else if (query == "Stop") {
            cin.ignore();  // пробел после Stop
            string stop_name;
            getline(cin, stop_name);

            res.push_back(make_unique<GetStopRequest>(move(stop_name)));
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
    vector<unique_ptr<ReadRequest>> read_requests = ParseReadRequests();

    for (const unique_ptr<ReadRequest> &read_request : read_requests) {
        cout << read_request->ServeRequest(db) << endl;
    }

}
