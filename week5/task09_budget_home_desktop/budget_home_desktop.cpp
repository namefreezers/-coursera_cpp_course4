#include <array>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;


struct Date {
    int year_, month_, day_;

//    istream& operator>>(istream& is) {
//        is >> year_;
//        is.get();
//        is >> month_;
//        is.get();
//        is >> day_;
//        return is;
//    }

    time_t AsTimestamp() const {
        tm t;
        t.tm_sec = 0;
        t.tm_min = 0;
        t.tm_hour = 0;
        t.tm_mday = day_;
        t.tm_mon = month_ - 1;
        t.tm_year = year_ - 1900;
        t.tm_isdst = 0;
        return mktime(&t);
    }
};

istream &operator>>(istream &is, Date &d) {
    is >> d.year_;
    is.get();
    is >> d.month_;
    is.get();
    is >> d.day_;
    return is;
}

int ComputeDaysDiff(const Date &date_to, const Date &date_from) {
    const time_t timestamp_to = date_to.AsTimestamp();
    const time_t timestamp_from = date_from.AsTimestamp();
    static const int SECONDS_IN_DAY = 60 * 60 * 24;
    return (timestamp_to - timestamp_from) / SECONDS_IN_DAY;
}

struct MonthDay {
    int month_;
    int day_;
};

class MonthData {
public:
    MonthData() {}

    MonthData(int days) : days_(days) {

    }

    void Earn(int day_from, int day_to, double value_per_day) {
        for (int d = day_from; d <= day_to; d++) {
            days_income_[d - 1] += value_per_day;
        }
    }

    void PayTax(int day_from, int day_to) {
        for (int d = day_from; d <= day_to; d++) {
            days_income_[d - 1] *= 0.87;
        }
    }

    double ComputeIncome(int day_from, int day_to) {
        double res = 0;
        for (int d = day_from; d <= day_to; d++) {
            res += days_income_[d - 1];
        }
        return res;
    }

private:
    array<double, 31> days_income_;
    int days_;
};

class YearData {
public:
    YearData() {}

    YearData(bool is_leap) : CURRENT_YEAR_MONTHS(is_leap ? LEAP_YEAR_MONTHS : NON_LEAP_YEAR_MONTHS) {
        for (int m = 0; m < 12; m++) {
            months_[m] = MonthData(get_month_days(m));
        }
    }

    int CalcFromForMonth(int m, const MonthDay &from) {
        if (m == from.month_) { return from.day_; }
        else { return 1; }
    }

    int CalcToForMonth(int m, const MonthDay &to) {
        if (m == to.month_) { return to.day_; }
        else { return CURRENT_YEAR_MONTHS[m - 1]; }
    }

    void Earn(const MonthDay &from, const MonthDay &to, double value_per_day) {
        for (int m = from.month_; m <= to.month_; m++) {
            int day_from = CalcFromForMonth(m, from), day_to = CalcToForMonth(m, to);

            months_[m - 1].Earn(day_from, day_to, value_per_day);
        }
    }

    void PayTax(const MonthDay &from, const MonthDay &to) {
        for (int m = from.month_; m <= to.month_; m++) {
            int day_from = CalcFromForMonth(m, from), day_to = CalcToForMonth(m, to);

            months_[m - 1].PayTax(day_from, day_to);
        }
    }

    double ComputeIncome(const MonthDay &from, const MonthDay &to) {
        double res = 0;
        for (int m = from.month_; m <= to.month_; m++) {
            int day_from = CalcFromForMonth(m, from), day_to = CalcToForMonth(m, to);

            res += months_[m - 1].ComputeIncome(day_from, day_to);
        }
        return res;
    }

private:
    static const array<uint8_t, 12> LEAP_YEAR_MONTHS;
    static const array<uint8_t, 12> NON_LEAP_YEAR_MONTHS;

    int get_month_days(int month) {
        return CURRENT_YEAR_MONTHS[month];
    }

    array<uint8_t, 12> CURRENT_YEAR_MONTHS;
    array<MonthData, 12> months_;

};


class BudgetManager {
public:
    BudgetManager() {
        for (int y = 2000; y < 2100; y++) {
            years_[y - 2000] = YearData(is_leap_year(y));
        }
    }

    MonthDay CalcFromForYear(int y, const Date &from) {
        if (y == from.year_) { return {from.month_, from.day_}; }
        else { return {1, 1}; }
    }

    MonthDay CalcToForYear(int y, const Date &to) {
        if (y == to.year_) { return {to.month_, to.day_}; }
        else { return {12, 31}; }
    }

    void Earn(const Date &from, const Date &to, double value) {
        int days = ComputeDaysDiff(to, from) + 1;
        for (int y = from.year_; y < to.year_ + 1; y++) {
            MonthDay month_day_from = CalcFromForYear(y, from);
            MonthDay month_day_to = CalcToForYear(y, to);

            years_[y - 2000].Earn(month_day_from, month_day_to, value / days);
        }
    }

    void PayTax(const Date &from, const Date &to) {
        for (int y = from.year_; y < to.year_ + 1; y++) {
            MonthDay month_day_from = CalcFromForYear(y, from);
            MonthDay month_day_to = CalcToForYear(y, to);

            years_[y - 2000].PayTax(month_day_from, month_day_to);
        }
    }

    double ComputeIncome(const Date &from, const Date &to) {
        double res = 0;
        for (int y = from.year_; y < to.year_ + 1; y++) {
            MonthDay month_day_from = CalcFromForYear(y, from);
            MonthDay month_day_to = CalcToForYear(y, to);

            res += years_[y - 2000].ComputeIncome(month_day_from, month_day_to);
        }
        return res;
    }

private:
    array<YearData, 100> years_;

    static bool is_leap_year(int y) {
        return (y % 400 == 0) || (y % 100 != 0 && y % 4 == 0);
    }
};

const array<uint8_t, 12> YearData::LEAP_YEAR_MONTHS = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const array<uint8_t, 12> YearData::NON_LEAP_YEAR_MONTHS = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int main() {
    cout.precision(25);

    BudgetManager manager;

    string command_Q;
    getline(cin, command_Q);

    int Q = stoi(command_Q);


    for (int i = 0; i < Q; i++) {
        string command;
        cin >> command;

        Date from, to;
        cin >> from >> to;

        if (command == "ComputeIncome") {
            cout << manager.ComputeIncome(from, to) << '\n';
        } else if (command == "Earn") {
            double value;
            cin >> value;

            manager.Earn(from, to, value);

        } else if (command == "PayTax") {
            manager.PayTax(from, to);
        }
    }
    return 0;
}
