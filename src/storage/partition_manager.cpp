#include "partition_manager.hpp"
#include <filesystem>

PartitionManager::PartitionManager(int num_partitions) {
    partitions_.reserve(num_partitions);
    for (size_t i = 0; i < num_partitions; ++i) {
        partitions_.push_back(std::make_unique<Partition>());
    }
}

std::optional<std::string> PartitionManager::Get(size_t idx, const std::string& key) const {
    std::lock_guard<userver::engine::SharedMutex> lock(mutex_);
    return partitions_[idx]->Get(key);
}

bool PartitionManager::Set(size_t idx, const std::string& key, const std::string& value, const uint64_t op_idx) {
    std::lock_guard<userver::engine::SharedMutex> lock(mutex_);
    return partitions_[idx]->Set(key, value, op_idx);
}

bool PartitionManager::Del(size_t idx, const std::string& key, const uint64_t op_idx) {
    std::lock_guard<userver::engine::SharedMutex> lock(mutex_);
    return partitions_[idx]->Del(key, op_idx);
}

void PartitionManager::SetNoLog(const size_t idx, const uint64_t op_idx, const std::string& key, const std::string& value) {
    std::lock_guard<userver::engine::SharedMutex> lock(mutex_);
    partitions_[idx]->SetNoLog(key, value, op_idx);
}

void PartitionManager::DelNoLog(const size_t idx, const uint64_t op_idx, const std::string& key) {
    std::lock_guard<userver::engine::SharedMutex> lock(mutex_);
    partitions_[idx]->DelNoLog(key, op_idx);
}

void PartitionManager::SnapshotAll(const std::string& dir) const {
    namespace fs = std::filesystem;
    fs::create_directories(dir);

    std::shared_lock<userver::engine::SharedMutex> lock(mutex_);
    for (size_t i = 0; i < partitions_.size(); ++i) {
        std::string filename = dir + "/partition_" + std::to_string(i) + ".snap";
        partitions_[i]->Snapshot(filename);
    }
}

void PartitionManager::RecoverAll(const std::string& dir) {
    namespace fs = std::filesystem;

    std::lock_guard<userver::engine::SharedMutex> lock(mutex_);
    for (size_t i = 0; i < partitions_.size(); ++i) {
        std::string filename = dir + "/partition_" + std::to_string(i) + ".snap";
        if (fs::exists(filename)) {
            partitions_[i]->Recover(filename);
        }
    }
}

void PartitionManager::ApplyOp(size_t idx, const AofOp& op) {
    std::lock_guard<userver::engine::SharedMutex> lock(mutex_);
    partitions_[idx]->ApplyOp(op);
}

size_t PartitionManager::Size() const {
    std::lock_guard<userver::engine::SharedMutex> lock(mutex_);
    return partitions_.size();
}
