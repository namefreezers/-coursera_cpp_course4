#include <iostream>
#include <numeric>
#include <utility>

#include "database.h"

using namespace std;

double Database::Stop::GetDistanceTo(const string &this_stop_name, const string &to_stop_name, const Database::Stop &to_stop) const {
    auto it = distances.find(to_stop_name);
    if (it != distances.end()) {
        return it->second;
    } else {
        return to_stop.distances.at(this_stop_name);
    }
}

void Database::AddStop(string name, Coords coords, unordered_map<string, double> distances) {
    stops.insert({move(name), {coords, move(distances), {}, static_cast<size_t>(-1)}});
}

void Database::AddBus(string bus_name, vector<string> stops_to_add) {
    size_t stops_amount = stops_to_add.size();
    size_t stops_amount_unique = CalculateUniqueStops(stops_to_add);
    double bus_coords_length = CalculateCoordsLength(stops_to_add);
    double bus_real_length = CalculateRealLength(stops_to_add);

    auto it = buses.insert({move(bus_name), {move(stops_to_add), stops_amount, stops_amount_unique, bus_coords_length, bus_real_length}});

    // add Bus to all Stops
    const vector<string> &stops_this_bus = it.first->second.stops;
    const string &this_bus_name = it.first->first;
    for (const string &stop : stops_this_bus) {
        stops.at(stop).stop_in_buses.insert(this_bus_name);
    }
}

void Database::ApplyFillRequests(DbInputRequests requests) {
    for (AddStopRequest &stop_req : requests.add_stop_requests) {
        AddStop(move(stop_req.stop_name), stop_req.coords, move(stop_req.distances));
    }
    for (AddBusRequest &bus_req : requests.add_bus_requests) {
        AddBus(move(bus_req.bus_name), move(bus_req.stops));
    }
}

double Database::CalculateRealLength(const vector<string> &bus_stops) const {
    if (bus_stops.size() <= 1) { return 0; }

    double real_length = 0;

    auto prev_stop_name_it = begin(bus_stops), next_stop_name_it = next(prev_stop_name_it);
    while (next_stop_name_it != bus_stops.end()) {
        real_length += stops.at(*prev_stop_name_it).GetDistanceTo(*prev_stop_name_it, *next_stop_name_it, stops.at(*next_stop_name_it));

        prev_stop_name_it++, next_stop_name_it++;
    }
    return real_length;
}

double Database::CalculateCoordsLength(const vector<string> &bus_stops) const {
    if (bus_stops.size() <= 1) { return 0; }

    double coords_length = 0;

    auto prev_stop_name_it = begin(bus_stops), next_stop_name_it = next(prev_stop_name_it);
    while (next_stop_name_it != bus_stops.end()) {
        double delta = stops.at(*prev_stop_name_it).coords - stops.at(*next_stop_name_it).coords;
        coords_length += delta;

        prev_stop_name_it++, next_stop_name_it++;
    }
    return coords_length;
}


void Database::AddEdgeFromStop(size_t vertex_id_stop, const string &stop_name, int bus_wait_time) {
    size_t from_stop_edge_id = graph->AddEdge({vertex_id_stop, vertex_id_stop + 1, static_cast<double>(bus_wait_time)});
    edges.push_back(make_pair(EdgeType::from_stop, make_unique<WaitRouteItem>(stop_name, bus_wait_time)));
}

void Database::AddBusEdge(const string &bus_name, const vector<string>& stops_in_bus, size_t from_stop_idx, size_t to_stop_idx, int bus_velocity) {
    double edge_time_res = 0;
    for (int i = from_stop_idx; i < to_stop_idx; ++i) {
        const string& from_stop_name = stops_in_bus.at(i);
        const string& next_stop_name = stops_in_bus.at(i+1);

        double bus_edge_time_minutes = stops.at(from_stop_name).GetDistanceTo(from_stop_name, next_stop_name, stops.at(next_stop_name)) / 1000 / bus_velocity * 60;
        edge_time_res += bus_edge_time_minutes;
    }

    size_t bus_edge_id = graph->AddEdge({stops.at(stops_in_bus.at(from_stop_idx)).id_in_graph + 1, stops.at(stops_in_bus.at(to_stop_idx)).id_in_graph, edge_time_res});

    edges.push_back(make_pair(EdgeType::bus_edge, make_unique<BusRouteItem>(bus_name, edge_time_res, to_stop_idx - from_stop_idx)));
}


void Database::FillRoutesGraph(const RoutingSettings &routing_settings) {
    graph = make_unique<Graph::DirectedWeightedGraph<double>>(stops.size() * 2);

    // fill id_in_graph for stops
    for (auto[i, it] = make_tuple(0, stops.begin()); it != stops.end(); it++, i += 2) {
        it->second.id_in_graph = i;

        AddEdgeFromStop(i, it->first, routing_settings.bus_wait_time);
    }

    for (const auto&[bus_name, bus] : buses) {

        for (size_t i = 0; i < bus.stops.size() - 1; i++) {
            for (size_t j = i + 1; j < bus.stops.size(); j++) {
                // ребра от "остановки в маршруте" до "остановки в маршруте"
                AddBusEdge(bus_name, bus.stops, i, j, routing_settings.bus_velocity);
            }
        }
    }

    router = make_unique<Graph::Router<double>>(*graph);
}


const Database::Stop *Database::GetStopInfo(const std::string &stop_name) const {
    auto it = stops.find(stop_name);
    if (it == stops.end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}

const Database::Bus *Database::GetBusInfo(const std::string &bus_name) const {
    auto it = buses.find(bus_name);
    if (it == buses.end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}

std::optional<Database::RouteInfoRes> Database::GetRouteInfo(const std::string &stop_from, const std::string &stop_to) const {
    std::optional<typename Graph::Router<double>::RouteInfo> route_info = router->BuildRoute(stops.at(stop_from).id_in_graph,
                                                                                             stops.at(stop_to).id_in_graph);
    if (!route_info.has_value()) {
        return nullopt;
    }
    vector<unique_ptr<RouteItem>> res;
    int current_edge_idx = 0;
    while (current_edge_idx < route_info->edge_count) {
        size_t edge_id = router->GetRouteEdge(route_info->id, current_edge_idx++);

        switch (edges[edge_id].first) {
            case EdgeType::from_stop: {
                unique_ptr<WaitRouteItem> item_ptr = make_unique<WaitRouteItem>(dynamic_cast<WaitRouteItem &>(*edges[edge_id].second));
                res.push_back(move(item_ptr));
                break;
            }
            case EdgeType::bus_edge: {
                unique_ptr<BusRouteItem> item_ptr = make_unique<BusRouteItem>(dynamic_cast<BusRouteItem &>(*edges[edge_id].second));
                res.push_back(move(item_ptr));
                break;
            }
            default:
                throw runtime_error("");
        }

    }

    router->ReleaseRoute(route_info->id);
    return Database::RouteInfoRes{move(res), route_info->weight};
}

size_t Database::CalculateUniqueStops(const vector<std::string> &stops) {
    set<string> stops_set(begin(stops), end(stops));
    return stops_set.size();
}

