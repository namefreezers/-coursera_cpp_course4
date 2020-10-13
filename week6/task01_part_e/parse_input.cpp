#include "parse_input.h"

using namespace std;

AddStopRequest ParseStopInputRequestJson(const Json::Node &stop_req) {
    string stop_name = stop_req.AsMap().at("name").AsString();
    double latitude = stop_req.AsMap().at("latitude").AsDouble();
    double longitude = stop_req.AsMap().at("longitude").AsDouble();
    unordered_map<string, double> distances;

    for (const auto&[k, v] : stop_req.AsMap().at("road_distances").AsMap()) {
        distances.insert({k, v.AsDouble()});
    }

    return AddStopRequest{move(stop_name), {latitude, longitude}, move(distances)};
}

AddBusRequest ParseBusInputRequestJson(const Json::Node &bus_req) {
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

    return AddBusRequest{move(bus_name), move(stops)};
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


// ===========================================================================================


unique_ptr<GetStopRequest> ParseReadStopRequestJson(const Json::Node &read_stop_req_node) {
    int id = static_cast<int>(read_stop_req_node.AsMap().at("id").AsDouble());
    string stop_name = read_stop_req_node.AsMap().at("name").AsString();

    return  make_unique<GetStopRequest>(id, move(stop_name));
}

std::unique_ptr<GetBusRequest>  ParseReadBusRequestJson(const Json::Node &read_bus_req_node) {
    int id = static_cast<int>(read_bus_req_node.AsMap().at("id").AsDouble());
    string bus_name = read_bus_req_node.AsMap().at("name").AsString();

    return make_unique<GetBusRequest>(id, move(bus_name));
}

std::unique_ptr<GetRouteRequest> ParseReadRouteRequestJson(const Json::Node &read_route_req_node) {
    int id = static_cast<int>(read_route_req_node.AsMap().at("id").AsDouble());
    string stop_from = read_route_req_node.AsMap().at("from").AsString();
    string stop_to = read_route_req_node.AsMap().at("to").AsString();

    return make_unique<GetRouteRequest>(id, move(stop_from), move(stop_to));
}

vector<unique_ptr<ReadRequest>> ParseReadRequestsJson(const Json::Node &read_requests) {
    vector<unique_ptr<ReadRequest>> res;

    for (const Json::Node &read_req_node : read_requests.AsArray()) {
        if (read_req_node.AsMap().at("type").AsString() == "Stop") {
            res.push_back(ParseReadStopRequestJson(read_req_node));
        } else if (read_req_node.AsMap().at("type").AsString() == "Bus") {
            res.push_back(ParseReadBusRequestJson(read_req_node));
        } else if (read_req_node.AsMap().at("type").AsString() == "Route") {
            res.push_back(ParseReadRouteRequestJson(read_req_node));
        } else {
            throw runtime_error("");
        }
    }

    return res;
}


// ===========================================================================================


RoutingSettings ParseRoutingSettingsJson(const Json::Node &routing_settings_node) {
    return RoutingSettings{
            static_cast<int>(routing_settings_node.AsMap().at("bus_wait_time").AsDouble()),
            static_cast<int>(routing_settings_node.AsMap().at("bus_velocity").AsDouble())
    };
}


// ===========================================================================================


tuple<DbInputRequests, vector<unique_ptr<ReadRequest>>, RoutingSettings> ParseRequestsJson(istream& is) {
    Json::Document doc = Json::Load(is);
    return make_tuple(
            ParseDbInputJson(doc.GetRoot().AsMap().at("base_requests")),
            ParseReadRequestsJson(doc.GetRoot().AsMap().at("stat_requests")),
            ParseRoutingSettingsJson(doc.GetRoot().AsMap().at("routing_settings"))
    );

}