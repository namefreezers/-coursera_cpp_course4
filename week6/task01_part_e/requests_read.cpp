#include <iomanip>
#include <sstream>

#include "requests_read.h"

using namespace std;


ReadRequest::ReadRequest(int id) : req_id(id) {}


string ReadRequest::ServeRequestByJsonData(const std::string &json_data, bool is_last_in_list)  const {
    stringstream ss;

    ss << "  {\n    \"request_id\": " << req_id << ",\n";
    ss << json_data;
    ss << "  }" << (is_last_in_list ? "" : ",") << "\n";

    return ss.str();
}


GetStopRequest::GetStopRequest(int id, std::string stop_name_) : stop_name(move(stop_name_)), ReadRequest(id) {}

std::string GetStopRequest::ServeRequestJson(const Database &db, bool is_last_in_list) const {
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


GetBusRequest::GetBusRequest(int id, string bus_name_) : bus_name(move(bus_name_)), ReadRequest(id) {}

std::string GetBusRequest::ServeRequestJson(const Database &db, bool is_last_in_list) const {

    stringstream ss;
    const Database::Bus *bus_info = db.GetBusInfo(bus_name);
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


GetRouteRequest::GetRouteRequest(int id, std::string stop_from_, std::string stop_to_) : stop_from(move(stop_from_)), stop_to(move(stop_to_)), ReadRequest(id) {}

std::string GetRouteRequest::ServeRequestJson(const Database &db, bool is_last_in_list) const {
    stringstream ss;

    optional <Database::RouteInfoRes> route_info_res = db.GetRouteInfo(stop_from, stop_to);
    if (!route_info_res.has_value()) {
        ss << "    \"error_message\": \"not found\"\n";
    } else {
        ss << "    \"total_time\": " << route_info_res->time << ",\n";
        ss << "    \"items\": [\n";
        if (!route_info_res->items.empty()) {
            for (auto it = route_info_res->items.begin(); it != prev(route_info_res->items.end()); it++) {
                ss << (*it)->GetInfoJson(6, false);
            }
            ss << (*prev(route_info_res->items.end()))->GetInfoJson(6, true); // without comma
        }
        ss << "    ]\n";

    }

    return ReadRequest::ServeRequestByJsonData(ss.str(), is_last_in_list);
}
