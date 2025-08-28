#include "storage.hpp"
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include <algorithm>

Storage::Storage(int num_partitions, const std::string& aof_path,
                 const std::string& snapshot_dir)
    : aof_logger_(aof_path), 
      snapshot_dir_(snapshot_dir),
      partitions_(num_partitions) {
    try {
        RecoverAdvanced(snapshot_dir_);
    } catch (const std::exception& e) {
        std::cerr << "RecoverAdvanced failed: " << e.what() << std::endl;
    }
}

size_t Storage::NumPartitions() const {
    return partitions_.Size();
}

size_t Storage::GetPartitionIndex(const std::string& key) const {
    std::hash<std::string> hasher;
    return hasher(key) % NumPartitions();
}

std::optional<std::string> Storage::Get(const std::string& key) const {
    auto idx = GetPartitionIndex(key);
    return partitions_.Get(idx, key);
}

bool Storage::Set(const std::string& key, const std::string& value) {
    auto idx = GetPartitionIndex(key);
    bool existed = partitions_.Set(idx, key, value);
    if (existed) {
        aof_logger_.Append(std::to_string(idx) + " SET " + key + " " + value);
    } 
    return existed; 
}

bool Storage::Del(const std::string& key) {
    auto idx = GetPartitionIndex(key);
    bool removed = partitions_.Del(idx, key);
    if (removed) {
        aof_logger_.Append(std::to_string(idx) + " DEL " + key);
    }
    return removed;
}

void Storage::Snapshot() const {
    partitions_.SnapshotAll(snapshot_dir_);
    std::cout << "Snapshot saved to " << snapshot_dir_ << std::endl;
}

AofLogger& Storage::GetAofLogger() {
    return aof_logger_;
}

void Storage::SetNoLog(const size_t partition_idx, const std::string& key, const std::string& value) {
    partitions_.SetNoLog(partition_idx, key, value);
}

void Storage::DelNoLog(const size_t partition_idx, const std::string& key) {
    partitions_.DelNoLog(partition_idx, key);
}

void Storage::ReplayAof() {
    auto ops = aof_logger_.ReadAll();
    for (const auto& op : ops) {
        partitions_.ApplyOp(op.partition_id, op);
    }
}

void Storage::RecoverAdvanced(const std::string& snapshots_dir) {
    namespace fs = std::filesystem;

    std::vector<std::chrono::system_clock::time_point> snapshot_times(NumPartitions());

    for (size_t i = 0; i < NumPartitions(); i++) {
        std::string filename = snapshots_dir + "/partition_" + std::to_string(i) + ".snap";
        if (!fs::exists(filename)) {
            throw std::runtime_error("Snapshot file not found: " + filename);
        }

        std::ifstream in(filename);
        if (!in.is_open()) {
            throw std::runtime_error("Failed to open snapshot file: " + filename);
        }

        std::string header_line;
        if (!std::getline(in, header_line)) {
            throw std::runtime_error("Empty snapshot file: " + filename);
        }

        std::istringstream iss(header_line);
        std::string ts_str;
        size_t count;
        if (!(iss >> ts_str >> count)) {
            throw std::runtime_error("Invalid snapshot header: " + filename);
        }

        snapshot_times[i] = ParseTimestampIso8601Z(ts_str);

        partitions_.ApplyOp(i, AofOp{}); // лишнее? но если Recover внутри PartitionManager, можно убрать
        partitions_.RecoverAll(snapshots_dir); // заменяем прямой вызов
    }

    auto min_snapshot_ts = *(std::min_element(snapshot_times.begin(), snapshot_times.end()));
    auto max_snapshot_ts = *(std::max_element(snapshot_times.begin(), snapshot_times.end()));

    auto ops = aof_logger_.ReadAll();

    for (const auto& op : ops) {
        if (op.ts < min_snapshot_ts) {
            continue; 
        }
        if (op.ts <= max_snapshot_ts) {
            if (op.ts > snapshot_times[op.partition_id]) {
                partitions_.ApplyOp(op.partition_id, op);
            }
        } else {
            partitions_.ApplyOp(op.partition_id, op);
        }
    }

    std::cout << "RecoverAdvanced finished. Applied AOF ops on top of snapshots.\n";
}
