#include "cache.h"

LRUCache::LRUCache(size_t capacity) : capacity_(capacity) {
    if (capacity_ == 0) capacity_ = 1;
}

bool LRUCache::get(const std::string &key, std::string &value_out) {
    //std::lock_guard<std::mutex> lock(mtx_);
    auto it = map_.find(key);
    if (it == map_.end()) {
        //std::cout<<"[CACHE MISS] key=" << key << std::endl;
        ++misses_;
        return false;
    }
    // moving the accessed item to front (most recent)
    //items_.splice(items_.begin(), items_, it->second);
    value_out = it->second->second;
    //std::cout<<"[CACHE HIT] key=" << key << std::endl;
    ++hits_;
    return true;
}

void LRUCache::put(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = map_.find(key);
    if (it != map_.end()) {
        // update and move to front
        it->second->second = value;
        items_.splice(items_.begin(), items_, it->second);
        return;
    }

    // evict item if cache capacity exceeded if needed
    if (items_.size() >= capacity_) {
        auto &last = items_.back();
        map_.erase(last.first);
        items_.pop_back();
    }

    items_.emplace_front(key, value);
    map_[key] = items_.begin();
}

bool LRUCache::erase(const std::string &key) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = map_.find(key);
    if (it == map_.end()) return false;
    items_.erase(it->second);
    map_.erase(it);
    return true;
}

void LRUCache::clear() {
    std::lock_guard<std::mutex> lock(mtx_);
    items_.clear();
    map_.clear();
    hits_ = 0;
    misses_ = 0;
}

size_t LRUCache::size() {
    std::lock_guard<std::mutex> lock(mtx_);
    return items_.size();
}

size_t LRUCache::capacity() const {
    return capacity_;
}

uint64_t LRUCache::hits() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return hits_;
}

uint64_t LRUCache::misses() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return misses_;
}

void LRUCache::reset_stats() {
    std::lock_guard<std::mutex> lock(mtx_);
    hits_ = 0;
    misses_ = 0;
}

