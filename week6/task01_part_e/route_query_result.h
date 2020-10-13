#pragma once

#include <memory>
#include <string>


class RouteItem {
public:
    virtual std::string GetInfoJson(int indent_size, bool is_last) const = 0;
};

class WaitRouteItem : public RouteItem {
public:
    WaitRouteItem(std::string stop_name_, int time_);

    std::string GetInfoJson(int indent_size, bool is_last) const override;

private:
    std::string stop_name;
    int time;
};

class BusRouteItem : public RouteItem {
public:
    BusRouteItem(std::string busName, double time, size_t spanCount = 1);

    std::string GetInfoJson(int indent_size, bool is_last) const override;

private:
    std::string bus_name;
    double time;
    size_t span_count;

};