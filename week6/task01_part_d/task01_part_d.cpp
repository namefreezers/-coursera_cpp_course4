#include <cmath>
#include <iostream>
#include <iomanip>
#include <memory>
#include <set>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "json.h"

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

        double GetDistanceTo(const string &this_stop_name, const string &to_stop_name, const Stop &to_stop) {
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

        const vector<string> &stops_this_bus = it.first->second.stops;
        const string this_bus_name = it.first->first;
        for (const string &stop : stops_this_bus) {
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


    const Stop *GetStopInfo(const string &stop_name) const {
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
    ReadRequest(int id) : req_id(id) {}

    int req_id;

    virtual string ServeRequestJson(const Database &, bool) = 0;

    string ServeRequestByJsonData(const string &json_data, bool is_last_in_list) {
        stringstream ss;

        ss << "  {\n    \"request_id\": " << req_id << ",\n";
        ss << json_data;
        ss << "  }" << (is_last_in_list ? "" : ",") << "\n";

        return ss.str();
    }
};

struct GetBusRequest : public ReadRequest {
    GetBusRequest(int id, string bus_name_) : bus_name(move(bus_name_)), ReadRequest(id) {}

    string bus_name;

    string ServeRequestJson(const Database &db, bool is_last_in_list) {

        stringstream ss;
        const Database::BusInfo *bus_info = db.GetBusInfo(bus_name);
        if (!bus_info) {
            ss << "    \"error_message\": \"not found\"\n";
        } else {
            ss << "    \"stop_count\": " << bus_info->num_stops << ",\n";
            ss << "    \"unique_stop_count\": " << bus_info->num_unique_stops << ",\n";
            ss << "    \"route_length\": " << setprecision(6) << bus_info->bus_real_length << ",\n";
            ss << "    \"curvature\": " << setprecision(6) << (bus_info->bus_real_length / bus_info->bus_calculated_length) << "\n";

        }
        return ReadRequest::ServeRequestByJsonData(ss.str(), is_last_in_list);
    }
};


struct GetStopRequest : public ReadRequest {
    GetStopRequest(int id, string stop_name_) : stop_name(move(stop_name_)), ReadRequest(id) {}

    string stop_name;

    string ServeRequestJson(const Database &db, bool is_last_in_list) {
        stringstream ss;
        const Database::Stop *stop = db.GetStopInfo(stop_name);

        if (!stop) {
            ss << "    \"error_message\": \"not found\"\n";
        } else {
            ss << "    \"buses\": [\n";
            if (!stop->stop_in_buses.empty()) {
                for (auto it = stop->stop_in_buses.begin(); it != prev(stop->stop_in_buses.end()); it++) {
                    ss << "      \"" << *it << "\",\n";
                }
                ss << "      \"" << *prev(stop->stop_in_buses.end()) << "\"\n"; // without comma
            }
            ss << "    ]\n";
        }
        return ReadRequest::ServeRequestByJsonData(ss.str(), is_last_in_list);
    }
};


unique_ptr<AddStopRequest> ParseStopInputRequestJson(const Json::Node &stop_req) {
    string stop_name = stop_req.AsMap().at("name").AsString();
    double latitude = stop_req.AsMap().at("latitude").AsDouble();
    double longitude = stop_req.AsMap().at("longitude").AsDouble();
    unordered_map<string, double> distances;

    for (const auto&[k, v] : stop_req.AsMap().at("road_distances").AsMap()) {
        distances.insert({k, v.AsDouble()});
    }

    return make_unique<AddStopRequest>(AddStopRequest{stop_name, {latitude, longitude}, move(distances)});
}

unique_ptr<AddBusRequest> ParseBusInputRequestJson(const Json::Node &bus_req) {
    string bus_name = bus_req.AsMap().at("name").AsString();
    bool is_circular = bus_req.AsMap().at("is_roundtrip").AsBool();
    vector<string> stops;

    for (const Json::Node &stop_node : bus_req.AsMap().at("stops").AsArray()) {
        stops.push_back(stop_node.AsString());
    }

    if (!is_circular) {  // зациклить
        for (int i = static_cast<int>(stops.size()) - 2; i >= 0; --i) {
            stops.push_back(stops[i]);
        }
    }

    return make_unique<AddBusRequest>(AddBusRequest{move(bus_name), move(stops)});
}


DbInputRequests ParseDbInputJson(const Json::Node &input_requests) {
    DbInputRequests res;

    for (const Json::Node &req_node : input_requests.AsArray()) {
        if (req_node.AsMap().at("type").AsString() == "Stop") {
            res.add_stop_requests.push_back(ParseStopInputRequestJson(req_node));
        } else if (req_node.AsMap().at("type").AsString() == "Bus") {
            res.add_bus_requests.push_back(ParseBusInputRequestJson(req_node));
        } else {
            throw runtime_error("");
        }
    }

    return res;
}

vector<unique_ptr<ReadRequest>> ParseReadRequestsJson(const Json::Node &read_requests) {
    vector<unique_ptr<ReadRequest>> res;

    for (const Json::Node &read_req_node : read_requests.AsArray()) {

        if (read_req_node.AsMap().at("type").AsString() == "Stop") {
            string stop_name = read_req_node.AsMap().at("name").AsString();
            int id = static_cast<int>(read_req_node.AsMap().at("id").AsDouble());

            res.push_back(make_unique<GetStopRequest>(id, move(stop_name)));
        } else if (read_req_node.AsMap().at("type").AsString() == "Bus") {
            string bus_name = read_req_node.AsMap().at("name").AsString();
            int id = static_cast<int>(read_req_node.AsMap().at("id").AsDouble());

            res.push_back(make_unique<GetBusRequest>(id, move(bus_name)));
        } else {
            throw runtime_error("");
        }
    }

    return res;
}


pair<DbInputRequests, vector<unique_ptr<ReadRequest>>> ParseRequestsJson() {
    Json::Document doc = Json::Load(cin);
    return make_pair(
            ParseDbInputJson(doc.GetRoot().AsMap().at("base_requests")),
            ParseReadRequestsJson(doc.GetRoot().AsMap().at("stat_requests"))
    );

}


int main() {
    Database db;

//    DbInputRequests db_input_requests = ParseDbInput();
//    vector<unique_ptr<ReadRequest>> read_requests = ParseReadRequests();

    pair<DbInputRequests, vector<unique_ptr<ReadRequest>>> requests = ParseRequestsJson();
    DbInputRequests db_input_requests = move(requests.first);
    vector<unique_ptr<ReadRequest>> read_requests = move(requests.second);

    // =========================================

    db.ApplyFillRequests(move(db_input_requests));

    db.CalculateBusLengthes();

    cout << "[\n";
    if (!read_requests.empty()) {
        for (int i = 0; i < read_requests.size() - 1; i++) {
            cout << read_requests[i]->ServeRequestJson(db, false);
        }
        cout << read_requests.back()->ServeRequestJson(db, true);
    }
    cout << "]\n";

}
