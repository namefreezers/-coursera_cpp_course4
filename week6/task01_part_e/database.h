#pragma once

#include <set>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>


#include "coords.h"
#include "graph.h"
#include "requests_input.h"
#include "route_query_result.h"
#include "router.h"
#include "routing_settings.h"


class Database {
public:
    struct Stop {
        Coords coords;
        std::unordered_map<std::string, double> distances;
        std::set<std::string> stop_in_buses;
        size_t id_in_graph;

        double GetDistanceTo(const std::string &this_stop_name, const std::string &to_stop_name, const Stop &to_stop) const;
    };

    struct Bus {
        std::vector<std::string> stops;
        size_t num_stops;
        size_t num_unique_stops;
        double bus_calculated_length;
        double bus_real_length;
    };

    enum class EdgeType {
        from_stop, bus_edge
    };

    struct RouteInfoRes {
        std::vector<std::unique_ptr<RouteItem>> items;
        double time;
    };

public:
    void AddStop(std::string name, Coords coords, std::unordered_map<std::string, double> distances);

    void AddBus(std::string bus_name, std::vector<std::string> stops_to_add);

    void ApplyFillRequests(DbInputRequests requests);

    void FillRoutesGraph(const RoutingSettings &routing_settings);

    const Stop *GetStopInfo(const std::string &stop_name) const;

    const Bus *GetBusInfo(const std::string &bus_name) const;

    std::optional<RouteInfoRes> GetRouteInfo(const std::string &stop_from, const std::string &stop_to) const;

private:


    std::unordered_map<std::string, Stop> stops;
    std::unordered_map<std::string, Bus> buses;

    std::unique_ptr<Graph::DirectedWeightedGraph<double>> graph;
    std::unique_ptr<Graph::Router<double>> router;
    std::vector<std::pair<EdgeType, std::unique_ptr<RouteItem>>> edges;

    double CalculateCoordsLength(const std::vector<std::string> &bus_stops) const;

    double CalculateRealLength(const std::vector<std::string> &bus_stops) const;

    void AddEdgeFromStop(size_t vertex_id_stop, const std::string &stop_name, int bus_wait_time);

    void AddBusEdge(const std::string &bus_name, const std::vector<std::string>& stops_in_bus, size_t from_stop_idx, size_t to_stop_idx, int bus_velocity);

    static size_t CalculateUniqueStops(const std::vector<std::string> &stops);
};
