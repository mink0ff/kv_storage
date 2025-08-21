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

void Partition::SetNoLog(const std::string& key, const std::string& value) {
    std::unique_lock lock(mutex_);
    kv_[key] = value;
}

void Partition::DelNoLog(const std::string& key) {
    std::unique_lock lock(mutex_);
    kv_.erase(key);
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

    out << GetTimestamp() << ' ' << copy.size() << "\n";
    for (const auto& [key, value] : copy) {
        out << key << "=" << value << "\n";
    }
}

void Partition::Recover(const std::string& filename) {
    std::lock_guard<std::shared_mutex> lock(mutex_);

    std::ifstream in(filename);
    if (!in.is_open()) {
        throw std::runtime_error("Failed to open snapshot file: " + filename);
    }

    std::string header;
    if (!std::getline(in, header)) {
        throw std::runtime_error("Snapshot file empty: " + filename);
    }

    // Заголовок нам нужен только для времени и количества — можно игнорировать

    kv_.clear();
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        kv_[std::move(key)] = std::move(value);
    }
}

void Partition::ApplyOp(const AofOp& op) {
    std::lock_guard<std::shared_mutex> lock(mutex_);

    if (op.type == AofOpType::SET) {
        kv_[op.key] = op.value;
    } else if (op.type == AofOpType::DEL) {
        kv_.erase(op.key);
    }
}

