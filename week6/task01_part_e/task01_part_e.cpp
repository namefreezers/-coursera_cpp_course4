#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <set>
#include <vector>
#include <unordered_map>

#include "database.h"
#include "parse_input.h"
#include "profile.h"
#include "requests_read.h"

using namespace std;




int main() {
    Database db;

//    auto opened_file = ifstream("../input/input4.txt");
//    tuple<DbInputRequests, vector<unique_ptr<ReadRequest>>, RoutingSettings> requests = ParseRequestsJson(opened_file);
    tuple<DbInputRequests, vector<unique_ptr<ReadRequest>>, RoutingSettings> requests = ParseRequestsJson(cin);
    DbInputRequests db_input_requests = move(get<0>(requests));
    vector<unique_ptr<ReadRequest>> read_requests = move(get<1>(requests));
    RoutingSettings routing_settings = get<2>(requests);

    // =========================================

    db.ApplyFillRequests(move(db_input_requests));


    db.FillRoutesGraph(routing_settings);

    cout << "[\n";
    if (!read_requests.empty()) {
        for (int i = 0; i < read_requests.size() - 1; i++) {
            cout << read_requests[i]->ServeRequestJson(db, false);
        }
        cout << read_requests.back()->ServeRequestJson(db, true);
    }
    cout << "]\n";

}
