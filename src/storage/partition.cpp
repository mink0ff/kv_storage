#include "partition.hpp"
#include <fstream>
#include <iostream>
#include <mutex>

std::optional<std::string> Partition::Get(const std::string& key) const {
    std::shared_lock lock(mutex_);
    auto it = kv_.find(key);
    if (it != kv_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool Partition::Set(const std::string& key, const std::string& value) {
    std::unique_lock lock(mutex_);
    kv_[key] = value;

    return true;
}

bool Partition::Del(const std::string& key) {
    std::unique_lock lock(mutex_);
    auto it = kv_.find(key);
    if (it == kv_.end()) {
        return false;
    }
    kv_.erase(it);

    return true;
}

void Partition::Snapshot(const std::string& filename) const {
    std::unordered_map<std::string, std::string> copy;

    {
        std::shared_lock lock(mutex_);
        copy = kv_; 
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл снапшота " << filename << std::endl;
        return;
    }

    out << copy.size() << "\n";
    for (const auto& [key, value] : copy) {
        out << key << "=" << value << "\n";
    }
}
