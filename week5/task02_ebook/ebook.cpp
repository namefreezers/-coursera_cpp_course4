#include <iomanip>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;

class ReadingManager {
public:
    ReadingManager() : amount_users_read_page(1001) {}

    void Read(int user_id, int page_count) {
        if (page_count == 0) {
            return;
        }

        if (user_page.count(user_id) == 0) {
            user_page[user_id] = 0;
            amount_users_read_page[0]++;
        }
        while (user_page.at(user_id) < page_count) {
            amount_users_read_page[++user_page.at(user_id)]++;
        }

    }

    double Cheer(int user_id) const {
        if (user_page.count(user_id) == 0) {
            return 0;
        }
        if (user_page.size() == 1) {  // единственный юзер
            return 1;
        }

        return (static_cast<int>(user_page.size())  - amount_users_read_page[user_page.at(user_id)]) * 1.0 / (static_cast<int>(user_page.size()) - 1);
    }

private:
    // Статическое поле не принадлежит какому-то конкретному
    // объекту класса. По сути это глобальная переменная,
    // в данном случае константная.
    // Будь она публичной, к ней можно было бы обратиться снаружи
    // следующим образом: ReadingManager::MAX_USER_COUNT.
    static const int MAX_USER_COUNT_ = 100'000;

    unordered_map<int, int> user_page;
    vector<int> amount_users_read_page;
};


int main() {
    // Для ускорения чтения данных отключается синхронизация
    // cin и cout с stdio,
    // а также выполняется отвязка cin от cout
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ReadingManager manager;

    int query_count;
    cin >> query_count;

    for (int query_id = 0; query_id < query_count; ++query_id) {
        string query_type;
        cin >> query_type;
        int user_id;
        cin >> user_id;

        if (query_type == "READ") {
            int page_count;
            cin >> page_count;
            manager.Read(user_id, page_count);
        } else if (query_type == "CHEER") {
            cout << setprecision(6) << manager.Cheer(user_id) << "\n";
        }
    }

    return 0;
}