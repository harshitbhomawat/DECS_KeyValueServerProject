#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <list>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <cstddef>

class LRUCache {
public:
    // capacity > 0
    explicit LRUCache(size_t capacity);

    // Try to get value for key. Returns true and sets value_out on hit.
    bool get(const std::string &key, std::string &value_out);

    // Insert or update a key -> value. If capacity exceeded, evict LRU item.
    void put(const std::string &key, const std::string &value);

    // Remove a key if present. Returns true if removed.
    bool erase(const std::string &key);

    // Clear all entries
    void clear();

    // Stats
    size_t size();
    size_t capacity() const;
    uint64_t hits() const;
    uint64_t misses() const;

private:
    using Item = std::pair<std::string, std::string>;
    std::list<Item> items_; // front = most recently used
    std::unordered_map<std::string, typename std::list<Item>::iterator> map_;
    size_t capacity_;
    mutable std::mutex mtx_;

    // stats
    uint64_t hits_ = 0;
    uint64_t misses_ = 0;
};

#endif // CACHE_H
