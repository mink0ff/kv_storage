#include "storage.hpp"
#include <filesystem>
#include <functional>
#include <iostream>

Storage::Storage(size_t num_partitions, const std::string& aof_path)
    : aof_logger_(aof_path) {
    partitions_.reserve(num_partitions);
    for (size_t i = 0; i < num_partitions; i++) {
        partitions_.emplace_back(std::make_unique<Partition>());
    }
}

size_t Storage::NumPartitions() const {
    return partitions_.size();
}

size_t Storage::GetPartitionIndex(const std::string& key) const {
    std::hash<std::string> hasher;
    return hasher(key) % NumPartitions();
}

std::optional<std::string> Storage::Get(const std::string& key) const {
    auto idx = GetPartitionIndex(key);
    return partitions_[idx]->Get(key);
}

bool Storage::Set(const std::string& key, const std::string& value) {
    auto idx = GetPartitionIndex(key);
    bool existed = partitions_[idx]->Set(key, value);
    if (existed) {
        aof_logger_.Append(std::to_string(idx) + " SET " + key + " " + value);
    } 
    return existed; 
}

bool Storage::Del(const std::string& key) {
    auto idx = GetPartitionIndex(key);
    bool removed = partitions_[idx]->Del(key);
    if (removed) {
        aof_logger_.Append(std::to_string(idx) + " DEL " + key);
    }
    return removed;
}

void Storage::Snapshot(const std::string& dir) const {
    namespace fs = std::filesystem;
    fs::create_directories(dir);

    for (size_t i = 0; i < partitions_.size(); i++) {
        std::string filename = dir + "/partition_" + std::to_string(i) + ".snap";
        partitions_[i]->Snapshot(filename);
    }
    std::cout << "Snapshot saved to " << dir << std::endl;
}
