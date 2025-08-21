#pragma once
#include "partition.hpp"
#include <vector>
#include <string>
#include <optional>
#include <memory>
#include "persistence/aof_logger.hpp"

class Storage {
public:
    explicit Storage(size_t num_partitions, const std::string& aof_path,
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


private:
    size_t NumPartitions() const;
    size_t GetPartitionIndex(const std::string& key) const;

    void SetNoLog(const size_t partition_idx, const std::string& key, const std::string& value);
    void DelNoLog(const size_t partition_idx, const std::string& key);

    AofLogger aof_logger_;
    std::string snapshot_dir_;

    std::vector<std::unique_ptr<Partition>> partitions_;
};
