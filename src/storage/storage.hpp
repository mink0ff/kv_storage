#pragma once

#include <string>
#include <optional>

#include "persistence/aof_logger.hpp"
#include "partition_manager.hpp"

class Storage {
public:
    explicit Storage(int num_partitions, const std::string& aof_path,
                     const std::string& snapshot_dir);

    // API
    std::optional<std::string> Get(const std::string& key) const;
    bool Set(const std::string& key, const std::string& value);
    bool Del(const std::string& key);

    // Сохранение снапшота
    void Snapshot() const;

    AofLogger& GetAofLogger();
    void ReplayAof();
    void RecoverAdvanced(const std::string& snapshots_dir);
    uint64_t NextOperationIndex();


private:
    size_t NumPartitions() const;
    size_t GetPartitionIndex(const std::string& key) const;

    void SetNoLog(const size_t partition_idx, const uint64_t op_idx, const std::string& key, const std::string& value);
    void DelNoLog(const size_t partition_idx, const uint64_t op_idx, const std::string& key);

    AofLogger aof_logger_;
    std::string snapshot_dir_;
    std::atomic_uint64_t operation_counter_{0};

    PartitionManager partitions_; // отдельный потокозащищенный класс Partitions 
};
