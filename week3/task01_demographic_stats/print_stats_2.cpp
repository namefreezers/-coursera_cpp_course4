#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nodiscard"

#include <algorithm>
#include <deque>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "test_runner.h"

using namespace std;

struct Person {
    string name;
    int age, income;
    bool is_male;
};

istream &operator>>(istream &input, Person &p) {
    input >> p.name >> p.age >> p.income;
    char gender;
    input >> gender;
    p.is_male = gender == 'M';
    return input;
}

deque<Person> ReadPeople(istream &input) {
    int count;
    input >> count;

    deque<Person> result(count);
    for (Person &p : result) {
        input >> p;
    }

    return result;
}

class PeopleDB {
public:
    PeopleDB(deque<Person>&& people_) : people(move(people_)) {

        // age
        for (const Person &p : people) {
            age_amount_ge[p.age]++;
        }
        if (!age_amount_ge.empty()) {
            for (auto latter_it = rbegin(age_amount_ge), earlier_it = next(latter_it); earlier_it != rend(age_amount_ge); latter_it++, earlier_it++) {
                earlier_it->second += latter_it->second;
            }
        }

        // income
        incomes_accumulated_from_largest.reserve(people.size());
        transform(begin(people), end(people), back_inserter(incomes_accumulated_from_largest), [](const Person &p) { return p.income; });
        sort(rbegin(incomes_accumulated_from_largest), rend(incomes_accumulated_from_largest));
        partial_sum(begin(incomes_accumulated_from_largest), end(incomes_accumulated_from_largest), begin(incomes_accumulated_from_largest));

        // popular name
        map<string, int> m_names, w_names;
        for (const Person &p : people) {
            if (p.is_male) {
                m_names[p.name]++;
            } else {
                w_names[p.name]++;
            }
        }

        if (m_names.empty()) {
            most_popular_m_name.reset();
        } else {
            most_popular_m_name = max_element(begin(m_names), end(m_names),
                                              [](const pair<string, int> &l, const pair<string, int> &r) { return l.second < r.second; })->first;
        }
        if (w_names.empty()) {
            most_popular_w_name.reset();
        } else {
            most_popular_w_name = max_element(begin(w_names), end(w_names),
                                              [](const pair<string, int> &l, const pair<string, int> &r) { return l.second < r.second; })->first;
        }
    }

    long people_maturity_age(int adult_age) const {
        auto it = age_amount_ge.lower_bound(adult_age);
        return it != end(age_amount_ge) ? it->second : 0;
    }

    long largest_N_income(int n) const {
        return n <= incomes_accumulated_from_largest.size() ? incomes_accumulated_from_largest[n - 1] : incomes_accumulated_from_largest.back();
    }

    std::optional<string> popular_m_name() const {
        return most_popular_m_name;
    }

    std::optional<string> popular_w_name() const {
        return most_popular_w_name;
    }

private:
    deque<Person> people;
    map<int, int> age_amount_ge;
    vector<long> incomes_accumulated_from_largest;
    std::optional<string> most_popular_m_name;
    std::optional<string> most_popular_w_name;
};

void Test500person10000name() {
    mt19937 engine;
    uniform_int_distribution char_dist(97, 122);
    uniform_int_distribution age_dist(1, 100);
    uniform_int_distribution income_dist(1, 100'000);
    bernoulli_distribution sex_dist;

    deque<Person> people(500);
    for (Person& p : people) {
        p.name = {static_cast<char>(char_dist(engine)), static_cast<char>(char_dist(engine))};
        p.age = age_dist(engine);
        p.income = income_dist(engine);
        p.is_male = sex_dist(engine);
    }
    const PeopleDB db(move(people));

    string m = db.popular_m_name().value(), w = db.popular_w_name().value();
    for (int i = 0; i < 100'000; i++) {
        bool is_m = sex_dist(engine);
        if (is_m) {
            if (db.popular_m_name().value() != m) {
                throw runtime_error("");
            }
        }
        else {
            if (db.popular_w_name().value() != w) {
                throw runtime_error("");
            }
        }
    }

}

int main() {
    {
        TestRunner tr;
        RUN_TEST(tr, Test500person10000name);
    }

    deque<Person> people = ReadPeople(cin);
    const PeopleDB db(move(people));

    for (string command; cin >> command;) {
        if (command == "AGE") {
            int adult_age;
            cin >> adult_age;

            cout << "There are " << db.people_maturity_age(adult_age) << " adult people for maturity age " << adult_age << '\n';
        } else if (command == "WEALTHY") {
            int count;
            cin >> count;

            cout << "Top-" << count << " people have total income " << db.largest_N_income(count) << '\n';
        } else if (command == "POPULAR_NAME") {
            char gender;
            cin >> gender;

            std::optional<string> most_popular_name = (gender == 'M') ? db.popular_m_name() : db.popular_w_name();

            if (!most_popular_name.has_value()) {
                cout << "No people of gender " << gender << '\n';
            } else {
                cout << "Most popular name among people of gender " << gender << " is "
                     << most_popular_name.value() << '\n';
            }
        }
    }
}

#pragma clang diagnostic pop