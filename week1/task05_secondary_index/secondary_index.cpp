#include "test_runner.h"

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace std;

struct Record {
    string id;
    string title;
    string user;
    int timestamp;
    int karma;

    Record() {}

    Record(string id, string title, string user, int timestamp, int karma) : id(move(id)),
                                                                             title(move(title)),
                                                                             user(move(user)),
                                                                             timestamp(timestamp),
                                                                             karma(karma) {}

    Record(Record &&other) : id(move(other.id)),
                             title(move(other.title)),
                             user(move(other.user)),
                             timestamp(other.timestamp),
                             karma(other.karma) {}

    Record(const Record &other) = default;
};

// Реализуйте этот класс
class Database {
public:
    bool Put(const Record &record) {
        bool inserted = id_record.find(record.id) == id_record.end();

        if (inserted) {
            auto *p_record = new Record(record);
            Record &inserted_record = *p_record;


            auto t_it = timestamp_id.insert({inserted_record.timestamp, p_record});
            auto k_it = karma_id.insert({inserted_record.karma, p_record});
            bool will_be_rehashed = user_id.bucket_count() * user_id.max_load_factor() < user_id.size() + 1;
            auto u_it = user_id.insert({inserted_record.user, p_record});

            id_record.insert({inserted_record.id, {p_record, t_it, k_it, u_it}});

            if (will_be_rehashed) {
                for (auto it = user_id.begin(); it != user_id.end(); it++) {
                    id_record.at(it->second->id).user_it = it;
                }
            }
        }
        return inserted;
    }

    const Record *GetById(const string &id) const {
        auto it = id_record.find(id);
        return it != id_record.end() ? it->second.p_record : nullptr;
    }

    bool Erase(const string &id) {
        auto it = id_record.find(id);
        bool erased = it != id_record.end();
        if (erased) {
            RecordInfo &erased_record_info = it->second;

            timestamp_id.erase(erased_record_info.timestamp_it);
            karma_id.erase(erased_record_info.karma_it);
            user_id.erase(erased_record_info.user_it);

            delete erased_record_info.p_record;
            id_record.erase(it);
        }
        return erased;
    }

    template<typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        auto it = timestamp_id.lower_bound(low);
        while (it != timestamp_id.end() && it->first <= high) {
            if (!callback(*it->second)) {
                return;
            }
            it++;
        }
    }

    template<typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        auto it = karma_id.lower_bound(low);
        while (it != karma_id.end() && it->first <= high) {
            if (!callback(*it->second)) {
                return;
            }
            it++;
        }
    }

    template<typename Callback>
    void AllByUser(const string &user, Callback callback) const {
        auto [it, it_end] = user_id.equal_range(user);
        while (it != it_end) {
            if (!callback(*it->second)) {
                return;
            }
            it++;
        }
    }

private:
    struct RecordInfo {
        Record *p_record;
        multimap<int, Record*>::iterator timestamp_it;
        multimap<int, Record*>::iterator karma_it;
        unordered_multimap<string_view, Record*>::iterator user_it;
    };

    unordered_map<string_view, RecordInfo> id_record;
    multimap<int, Record*> timestamp_id, karma_id;
    unordered_multimap<string_view, Record*> user_id;
};

void TestRangeBoundaries() {
    const int good_karma = 1000;
    const int bad_karma = -10;

    Database db;
    db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
    db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

    int count = 0;
    db.RangeByKarma(bad_karma, good_karma, [&count](const Record &) {
        ++count;
        return true;
    });

    ASSERT_EQUAL(2, count);
}

void TestSameUser() {
    Database db;
    db.Put({"id1", "Don't sell", "master", 1536107260, 1000});
    db.Put({"id2", "Rethink life", "master", 1536107260, 2000});

    int count = 0;
    db.AllByUser("master", [&count](const Record &) {
        ++count;
        return true;
    });

    ASSERT_EQUAL(2, count);
}

void TestReplacement() {
    const string final_body = "Feeling sad";

    Database db;
    db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
    db.Erase("id");
    db.Put({"id", final_body, "not-master", 1536107260, -10});

    auto record = db.GetById("id");
    ASSERT(record != nullptr);
    ASSERT_EQUAL(final_body, record->title);
}

int main() {

    TestRunner tr;
    RUN_TEST(tr, TestRangeBoundaries);
    RUN_TEST(tr, TestSameUser);
    RUN_TEST(tr, TestReplacement);
    return 0;
}
