#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class RouteManager {
public:
    void AddRoute(int start, int finish) {
        reachable_lists_[start].insert(finish);
        reachable_lists_[finish].insert(start);
    }

    int FindNearestFinish(int start, int finish) const {
        int DistanceFromStart = abs(start - finish);
        if (reachable_lists_.count(start) < 1) {
            return DistanceFromStart;
        }

        int result = min(
                DistanceFromStart,
                FindResultNearestExpress(start, finish, reachable_lists_.at(start))
        );

        return result;
    }

private:
    unordered_map<int, set<int>> reachable_lists_;

    int FindResultNearestExpress(int start, int finish, const set<int> &reachable_stations) const {
        int nearest_express_result;

        auto upper_it = reachable_stations.upper_bound(finish);
        if (upper_it == reachable_stations.end()) {
            nearest_express_result = abs(*prev(upper_it) - finish);
        } else if (upper_it == reachable_stations.begin()) {
            nearest_express_result = abs(*upper_it - finish);
        } else {
            nearest_express_result = min(
                    abs(*prev(upper_it) - finish),
                    abs(*upper_it - finish)
            );
        }
        return nearest_express_result;
    }
};


int main() {
    RouteManager routes;

    int query_count;
    cin >> query_count;

    for (int query_id = 0; query_id < query_count; ++query_id) {
        string query_type;
        cin >> query_type;
        int start, finish;
        cin >> start >> finish;
        if (query_type == "ADD") {
            routes.AddRoute(start, finish);
        } else if (query_type == "GO") {
            cout << routes.FindNearestFinish(start, finish) << "\n";
        }
    }

    return 0;
}
