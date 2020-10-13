#include "route_query_result.h"

#include <iomanip>
#include <sstream>
#include <utility>

using namespace std;

WaitRouteItem::WaitRouteItem(string stop_name_, int time_) : stop_name(move(stop_name_)), time(time_) {}

string WaitRouteItem::GetInfoJson(int indent_size, bool is_last) const {
    stringstream ss;
    ss << string(indent_size, ' ') << "{\n";
    ss << string(indent_size, ' ') << "  \"type\": \"Wait\",\n";
    ss << string(indent_size, ' ') << "  \"time\": " << time << ",\n";
    ss << string(indent_size, ' ') << "  \"stop_name\": \"" << stop_name << "\"\n";
    ss << string(indent_size, ' ') << "}" << (is_last ? "" : ",") << "\n";
    return ss.str();
}

BusRouteItem::BusRouteItem(string busName, double time, size_t spanCount) : bus_name(move(busName)), span_count(spanCount), time(time) {}

string BusRouteItem::GetInfoJson(int indent_size, bool is_last) const {
    stringstream ss;
    ss << string(indent_size, ' ') << "{\n";
    ss << string(indent_size, ' ') << "  \"type\": \"Bus\",\n";
    ss << string(indent_size, ' ') << "  \"time\": " << setprecision(6) << time << ",\n";
    ss << string(indent_size, ' ') << "  \"bus\": \"" << bus_name << "\",\n";
    ss << string(indent_size, ' ') << "  \"span_count\": " << span_count << "\n";
    ss << string(indent_size, ' ') << "}" << (is_last ? "" : ",") << "\n";
    return ss.str();
}
