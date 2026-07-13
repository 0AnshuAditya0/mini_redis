#include "store.hpp"

void Store::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    data_[key] = value;
}

std::optional<std::string> Store::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it == data_.end()) return std::nullopt;
    return it->second;
}

bool Store::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.erase(key) > 0;
}