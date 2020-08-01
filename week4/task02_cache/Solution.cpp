#include <algorithm>
#include <mutex>
#include <unordered_map>

#include "Common.h"

using namespace std;

class LruCache : public ICache {
public:
    LruCache(shared_ptr<IBooksUnpacker> books_unpacker, const Settings &settings) : books_unpacker_(move(books_unpacker)), settings_(settings) {
        // реализуйте метод
    }

    BookPtr GetBook(const string &book_name) override {
        {
            auto lg = lock_guard<mutex>(m);
            if (auto cache_it = cache.find(book_name); cache_it != cache.end()) {
                cache_it->second.rank = last_rank++;
                return cache_it->second.book;
            }
        }
        BookPtr book_ptr = books_unpacker_->UnpackBook(book_name);

        {
            auto lg = lock_guard<mutex>(m);
            while (!cache.empty() && current_size + book_ptr->GetContent().size() > settings_.max_memory) {
                auto it_to_remove = min_element(cache.begin(), cache.end(),
                                                [](const pair<string, CacheItem> &lhs, const pair<string, CacheItem> &rhs) { return lhs.second < rhs.second; });
                current_size -= it_to_remove->second.book->GetContent().size();
                cache.erase(it_to_remove);

            }
            if (current_size + book_ptr->GetContent().size() <= settings_.max_memory) {
                cache[book_ptr->GetName()] = {book_ptr, last_rank++};
                current_size += book_ptr->GetContent().size();
            }
        }
        return book_ptr;
    }

private:
    struct CacheItem {
        BookPtr book;
        size_t rank;

        bool operator<(const CacheItem &other) const {
            return this->rank < other.rank;
        }
    };

    shared_ptr<IBooksUnpacker> books_unpacker_;
    Settings settings_;
    unordered_map<string, CacheItem> cache;
    size_t last_rank = 0, current_size = 0;
    mutex m;

};


unique_ptr<ICache> MakeCache(
        shared_ptr<IBooksUnpacker> books_unpacker,
        const ICache::Settings &settings
) {
    return make_unique<LruCache>(move(books_unpacker), settings);
}
