#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

using namespace std;

enum class Gender {
    FEMALE,
    MALE
};

struct Person {
    int age;  // возраст
    Gender gender;  // пол
    bool is_employed;  // имеет ли работу
};

template<typename InputIt>
int ComputeMedianAge(InputIt range_begin, InputIt range_end) {
    if (range_begin == range_end) {
        return 0;
    }
    vector<typename InputIt::value_type> range_copy(range_begin, range_end);
    auto middle = begin(range_copy) + range_copy.size() / 2;
    nth_element(
            begin(range_copy), middle, end(range_copy),
            [](const Person &lhs, const Person &rhs) {
                return lhs.age < rhs.age;
            }
    );
    return middle->age;
}

void PrintStatsForGroup(vector<Person> &persons, string for_group, function<bool(const Person &)> p) {
    auto end_part_it = partition(persons.begin(), persons.end(), p);
    cout << "Median age" << for_group << " = " << ComputeMedianAge(persons.begin(), end_part_it) << endl;
}

void PrintStats(vector<Person> persons) {
    PrintStatsForGroup(persons, "", [](const Person &p) { return true; });
    PrintStatsForGroup(persons, " for females", [](const Person &p) { return p.gender == Gender::FEMALE; });
    PrintStatsForGroup(persons, " for males", [](const Person &p) { return p.gender == Gender::MALE; });
    PrintStatsForGroup(persons, " for employed females", [](const Person &p) { return p.is_employed && p.gender == Gender::FEMALE; });
    PrintStatsForGroup(persons, " for unemployed females", [](const Person &p) { return !p.is_employed && p.gender == Gender::FEMALE; });
    PrintStatsForGroup(persons, " for employed males", [](const Person &p) { return p.is_employed && p.gender == Gender::MALE; });
    PrintStatsForGroup(persons, " for unemployed males", [](const Person &p) { return !p.is_employed && p.gender == Gender::MALE; });
}

int main() {
    vector<Person> persons = {
            {31, Gender::MALE,   false},
            {40, Gender::FEMALE, true},
            {24, Gender::MALE,   true},
            {20, Gender::FEMALE, true},
            {80, Gender::FEMALE, false},
            {78, Gender::MALE,   false},
            {10, Gender::FEMALE, false},
            {55, Gender::MALE,   true},
    };
    PrintStats(persons);
    return 0;
}
