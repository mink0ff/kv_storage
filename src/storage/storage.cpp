#include "storage.hpp"
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <userver/logging/log.hpp>
#include "utils/utils.hpp"


Storage::Storage(int num_partitions, const std::string& aof_path,
                 const std::string& snapshot_dir)
    : aof_logger_(aof_path), 
      snapshot_dir_(snapshot_dir),
      operation_counter_(0),
      partitions_(num_partitions)
{
    try {
        RecoverAdvanced(snapshot_dir_);
    } catch (const std::exception& e) {
        std::cerr << "RecoverAdvanced failed: " << e.what() << std::endl;
    }
}

Storage::~Storage() {
    try {
        Snapshot();
    } catch (const std::exception& e) {
        std::cerr << "Snapshot in destructor failed: " << e.what() << std::endl;
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
    uint64_t op_idx = NextOperationIndex();

    bool existed = partitions_.Set(idx, key, value, op_idx);
    
    if (existed) {
        AofOp op {
        .ts = std::chrono::system_clock::now(),
        .operation_idx = op_idx,
        .partition_id = idx,
        .type = AofOpType::SET,
        .key = key,
        .value = value
    };
        aof_logger_.Append(op);
    } 
    return existed; 
}

bool Storage::Del(const std::string& key) {
    auto idx = GetPartitionIndex(key);
    uint64_t op_idx = NextOperationIndex();

    bool removed = partitions_.Del(idx, key, op_idx);
    if (removed) {
        AofOp op {
        .ts = std::chrono::system_clock::now(),
        .operation_idx = op_idx,
        .partition_id = idx,
        .type = AofOpType::DEL,
        .key = key
        };
        aof_logger_.Append(op);
    }
    return removed;
}

void Storage::Snapshot() const {
    partitions_.SnapshotAll(snapshot_dir_);
    LOG_INFO() << "Snapshot saved to " << snapshot_dir_;
}

AofLogger& Storage::GetAofLogger() {
    return aof_logger_;
}

void Storage::SetNoLog(const size_t partition_idx, const uint64_t op_idx, const std::string& key, const std::string& value) {
    partitions_.SetNoLog(partition_idx, op_idx, key, value);
}

void Storage::DelNoLog(const size_t partition_idx, const uint64_t op_idx, const std::string& key) {
    partitions_.DelNoLog(partition_idx, op_idx, key);
}

void Storage::ReplayAof() {
    auto ops = aof_logger_.ReadAll();
    for (const auto& op : ops) {
        partitions_.ApplyOp(op.partition_id, op);
    }
}

uint64_t Storage::NextOperationIndex() {
        return operation_counter_.fetch_add(1, std::memory_order_relaxed);
    }

void Storage::RecoverAdvanced(const std::string& snapshots_dir) {
    namespace fs = std::filesystem;

    std::vector<uint64_t> snapshot_last_op_idxs(NumPartitions());

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
        std::string partition_last_op_idx;
        size_t count;
        if (!(iss >> ts_str >> partition_last_op_idx >> count)) {
            throw std::runtime_error("Invalid snapshot header: " + filename);
        }

        snapshot_last_op_idxs[i] = std::stoull(partition_last_op_idx);

        partitions_.ApplyOp(i, AofOp{}); 
        partitions_.RecoverAll(snapshots_dir);
    }

    auto min_snapshot_op_idx = *(std::min_element(snapshot_last_op_idxs.begin(), snapshot_last_op_idxs.end()));
    auto max_snapshot_op_idx = *(std::max_element(snapshot_last_op_idxs.begin(), snapshot_last_op_idxs.end()));

    auto ops = aof_logger_.ReadAll();

    if (ops.empty()) {
        LOG_WARNING() << "No AOF operations to replay.\n";
        return;
    }

    for (const auto& op : ops) {
        if (op.operation_idx < min_snapshot_op_idx) {
            continue; 
        }
        if (op.operation_idx <= max_snapshot_op_idx) {
            if (op.operation_idx > snapshot_last_op_idxs[op.partition_id]) {
                partitions_.ApplyOp(op.partition_id, op);
            }
        } else {
            partitions_.ApplyOp(op.partition_id, op);
        }
    }
    operation_counter_.store(ops.back().operation_idx + 1, std::memory_order_relaxed);

    LOG_INFO() << "RecoverAdvanced finished. Applied AOF ops on top of snapshots.";
}
