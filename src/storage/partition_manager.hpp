#pragma once

#include <vector>
#include <memory>
#include <string>
#include <optional>

#include <userver/engine/shared_mutex.hpp>

#include "partition.hpp"
#include "../persistence/aof_logger.hpp" 

class PartitionManager {
public:
    explicit PartitionManager(int num_partitions);

    std::optional<std::string> Get(size_t idx, const std::string& key) const;
    bool Set(size_t idx, const std::string& key, const std::string& value, const uint64_t op_idx);
    bool Del(size_t idx, const std::string& key, const uint64_t op_idx);

    void SetNoLog(const size_t idx, const uint64_t op_idx, const std::string& key, const std::string& value);
    void DelNoLog(const size_t idx, const uint64_t op_idx, const std::string& key);

    void SnapshotAll(const std::string& dir) const;
    void RecoverAll(const std::string& dir);

    void ApplyOp(size_t idx, const AofOp& op);

    size_t Size() const;

private:
    mutable userver::engine::SharedMutex mutex_;
    std::vector<std::unique_ptr<Partition>> partitions_;
};
