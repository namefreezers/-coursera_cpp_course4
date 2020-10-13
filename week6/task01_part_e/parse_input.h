#pragma once

#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

#include "json.h"
#include "requests_input.h"
#include "requests_read.h"
#include "routing_settings.h"

AddStopRequest ParseStopInputRequestJson(const Json::Node &stop_req);

AddBusRequest ParseBusInputRequestJson(const Json::Node &bus_req);

DbInputRequests ParseDbInputJson(const Json::Node &input_requests);

// ===========================================================================================

std::unique_ptr<GetStopRequest> ParseReadStopRequestJson(const Json::Node &stop_req);

std::unique_ptr<GetBusRequest>  ParseReadBusRequestJson(const Json::Node &stop_req);

std::unique_ptr<GetRouteRequest> ParseReadRouteRequestJson(const Json::Node &stop_req);

std::vector<std::unique_ptr<ReadRequest>> ParseReadRequestsJson(const Json::Node &read_requests);

// ===========================================================================================

RoutingSettings ParseRoutingSettingsJson(const Json::Node &routing_settings_node);

// ===========================================================================================

std::tuple<DbInputRequests, std::vector<std::unique_ptr<ReadRequest>>, RoutingSettings> ParseRequestsJson(std::istream &is);
