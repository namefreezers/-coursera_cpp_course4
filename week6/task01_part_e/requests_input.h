#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "coords.h"


struct AddStopRequest {
    std::string stop_name;
    Coords coords;
    std::unordered_map<std::string, double> distances;
};

struct AddBusRequest {
    std::string bus_name;
    std::vector<std::string> stops;
};

struct DbInputRequests {
    std::vector<AddStopRequest> add_stop_requests;
    std::vector<AddBusRequest> add_bus_requests;
};
