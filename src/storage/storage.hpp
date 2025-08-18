#pragma once
#include "partition.hpp"
#include <vector>
#include <string>
#include <optional>
#include <memory>

class Storage {
public:
    explicit Storage(size_t num_partitions);

    // API
    std::optional<std::string> Get(const std::string& key) const;
    bool Set(const std::string& key, const std::string& value);
    bool Del(const std::string& key);

    // Сохранение снапшота
    void Snapshot(const std::string& dir) const;

private:
    size_t NumPartitions() const;
    size_t GetPartitionIndex(const std::string& key) const;

    std::vector<std::unique_ptr<Partition>> partitions_;
};
