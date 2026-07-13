#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>

class Store {
public:
    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);

private:
    std::unordered_map<std::string, std::string> data_;
    std::mutex mutex_;
};