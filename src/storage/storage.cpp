#include "storage.hpp"
#include <filesystem>
#include <functional>
#include <iostream>

Storage::Storage(size_t num_partitions, const std::string& aof_path,
                 const std::string& snapshot_dir)
    : aof_logger_(aof_path), 
      snapshot_dir_(snapshot_dir) {
    partitions_.reserve(num_partitions);
    for (size_t i = 0; i < num_partitions; i++) {
        partitions_.emplace_back(std::make_unique<Partition>());
    }

    // При создании Storage сразу пытаемся восстановиться
    try {
        RecoverAdvanced(snapshot_dir_);
    } catch (const std::exception& e) {
        std::cerr << "RecoverAdvanced failed: " << e.what() << std::endl;
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

void Storage::Snapshot() const {
    namespace fs = std::filesystem;
    fs::create_directories(snapshot_dir_);

    for (size_t i = 0; i < partitions_.size(); i++) {
        std::string filename = snapshot_dir_ + "/partition_" + std::to_string(i) + ".snap";
        partitions_[i]->Snapshot(filename);
    }
    std::cout << "Snapshot saved to " << snapshot_dir_ << std::endl;
}

AofLogger& Storage::GetAofLogger() {
    return aof_logger_;
}

void Storage::SetNoLog(const size_t partition_idx, const std::string& key, const std::string& value) {
    partitions_[partition_idx]->SetNoLog(key, value);
}

void Storage::DelNoLog(const size_t partition_idx, const std::string& key) {
    partitions_[partition_idx]->DelNoLog(key);
}

void Storage::ReplayAof() {
    
};

void Storage::RecoverAdvanced(const std::string& snapshots_dir) {
    namespace fs = std::filesystem;

    std::vector<std::chrono::system_clock::time_point> snapshot_times(partitions_.size());

    for (size_t i = 0; i < partitions_.size(); i++) {
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

        partitions_[i]->Recover(filename);
    }

    auto min_snapshot_ts = *(std::min_element(snapshot_times.begin(), snapshot_times.end()));
    auto max_snapshot_ts = *(std::max_element(snapshot_times.begin(), snapshot_times.end()));

    auto ops = aof_logger_.ReadAll();

    // Делим операции на три диапазона:
    // [< min_snapshot_ts]  -> пропускаем
    // [min_snapshot_ts .. max_snapshot_ts] -> проверяем по времени партиции
    // [> max_snapshot_ts] -> применяем без проверок
    for (const auto& op : ops) {
        if (op.ts < min_snapshot_ts) {
            continue; 
        }
        if (op.ts <= max_snapshot_ts) {
            if (op.ts > snapshot_times[op.partition_id]) {
                partitions_[op.partition_id]->ApplyOp(op);
            }
        } else {
            partitions_[op.partition_id]->ApplyOp(op);
        }
    }

    std::cout << "RecoverAdvanced finished. Applied AOF ops on top of snapshots.\n";
}