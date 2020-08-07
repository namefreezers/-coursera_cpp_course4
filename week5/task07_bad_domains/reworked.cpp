#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using namespace std;


bool IsSubdomain(const string_view subdomain, const string_view domain) {
    int i = subdomain.size() - 1;
    int j = domain.size() - 1;
    while (i >= 0 && j >= 0) {
        if (subdomain[i--] != domain[j--]) {
            return false;
        }
    }
    return (i < 0 && domain[j] == '.')
           || (j < 0 && subdomain[i] == '.')
           || (i < 0 && j < 0);
}


vector<string> ReadDomains() {
    size_t count;
    cin >> count;

    vector<string> domains;
    domains.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        string domain;
        cin >> domain;
        domains.push_back(move(domain));
    }
    return domains;
}


struct ReversedComparison {
    bool operator()(const string &lhs, const string &rhs) const {
        return lexicographical_compare(lhs.rbegin(), lhs.rend(), rhs.rbegin(), rhs.rend());
    }
};

set<string, ReversedComparison> MakeSetFromVector(vector<string> domains) {
    set<string, ReversedComparison> res(make_move_iterator(domains.begin()), make_move_iterator(domains.end()));
    if (res.size() < 2) {
        return res;
    }

    for (auto prev_it = res.begin(), next_it = next(res.begin()); next_it != res.end(); ) {
        if (IsSubdomain(*prev_it, *next_it)) {
            next_it = res.erase(next_it);
        } else {
            prev_it++, next_it++;
        }
    }

    return res;
}


int main() {
    const set<string, ReversedComparison> banned_domains_set = MakeSetFromVector(ReadDomains());
    const vector<string> domains_to_check = ReadDomains();


    for (const string &domain_to_check : domains_to_check) {
        auto upper_it = banned_domains_set.upper_bound(domain_to_check);
        if (upper_it != banned_domains_set.begin() && IsSubdomain(*prev(upper_it), domain_to_check)) {
            cout << "Bad" << endl;
        } else {
            cout << "Good" << endl;
        }
    }

    return 0;
}
