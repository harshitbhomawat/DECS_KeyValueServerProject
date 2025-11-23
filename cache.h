#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <list>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <cstddef>
#include <iostream>

class LRUCache {
public:
    explicit LRUCache(size_t capacity);

    bool get(const std::string &key, std::string &value_out);

    void put(const std::string &key, const std::string &value);

    bool erase(const std::string &key);

    void clear();

    size_t size();
    size_t capacity() const;
    uint64_t hits() const;
    uint64_t misses() const;
    void reset_stats();

private:
    using Item = std::pair<std::string, std::string>;
    std::list<Item> items_; // front = most recently used
    std::unordered_map<std::string, typename std::list<Item>::iterator> map_;
    size_t capacity_;
    mutable std::mutex mtx_;

    // cache performance statistics
    uint64_t hits_ = 0;
    uint64_t misses_ = 0;
};

#endif
