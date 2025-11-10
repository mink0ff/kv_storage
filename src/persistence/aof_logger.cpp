#include "aof_logger.hpp"
#include <iostream>
#include "utils/utils.hpp"
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/logging/log.hpp>





AofLogger::AofLogger(const std::string& filename): 
filename_(filename) {
    file_.open(filename, std::ios::app);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open AOF file: " + filename);
    }
}


AofLogger::~AofLogger() {
    if (file_.is_open()) {
        Flush();   
        file_.close(); 
    }
}

void AofLogger::Append(const AofOp& op) {
    std::lock_guard<userver::engine::Mutex> lock(mutex_);

    userver::formats::json::ValueBuilder json;
    json["ts"] = TimePointToIsoString(op.ts);
    json["operation_idx"] = op.operation_idx;
    json["partition_id"] = op.partition_id;
    json["type"] = (op.type == AofOpType::SET) ? "SET" : "DEL";
    json["key"] = op.key;
    if (op.type == AofOpType::SET) {
        json["value"] = op.value;
    }
    file_ << userver::formats::json::ToString(json.ExtractValue()) << "\n";
    file_.flush();  
}

void AofLogger::Flush() {
    std::lock_guard<userver::engine::Mutex> lock(mutex_);
    file_.flush();
}

void AofLogger::Clear() {
    std::lock_guard<userver::engine::Mutex> lock(mutex_);

    if (file_.is_open()) {
        file_.close();
    }

    file_.open(filename_, std::ios::trunc);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to truncate AOF file: " + filename_);
    }

    file_.close();
    file_.open(filename_, std::ios::app);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to reopen AOF file: " + filename_);
    }
}

std::vector<AofOp> AofLogger::ReadAll() const {
    return ReadFile(filename_);
}

std::vector<AofOp> AofLogger::ReadFile(const std::string& path) const {
    std::vector<AofOp> ops;
    std::ifstream in(path);
    if (!in.is_open()) return ops;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
    //     std::istringstream iss(line);

    //     std::string ts_str, op_str;
    //     std::string op_idx; 
    //     std::size_t pid;
    //     if (!(iss >> ts_str >> op_idx >> pid >> op_str)) continue;

    //     AofOp op;
    //     op.ts = ParseTimestampIso8601Z(ts_str);
    //     op.operation_idx = std::stoull(op_idx);
    //     op.partition_id = pid;

    //     if (op_str == "SET") {
    //         std::string key, value;
    //         if (!(iss >> key >> value)) continue;
    //         op.type = AofOpType::SET;
    //         op.key = std::move(key);
    //         op.value = std::move(value);
    //     } else if (op_str == "DEL") {
    //         std::string key;
    //         if (!(iss >> key)) continue;
    //         op.type = AofOpType::DEL;
    //         op.key = std::move(key);
    //     } else {
    //         continue;
    //     }
    //     ops.emplace_back(std::move(op));
    // }
        try {
            auto json = userver::formats::json::FromString(line);
            AofOp op;

            op.ts = ParseTimestampIso8601Z(json["ts"].As<std::string>());
            op.operation_idx = json["operation_idx"].As<std::uint64_t>();
            op.partition_id = json["partition_id"].As<std::size_t>();
            const auto op_str = json["type"].As<std::string>();
            op.key = json["key"].As<std::string>();

            if (op_str == "SET") {
                op.type = AofOpType::SET;
                op.value = json["value"].As<std::string>();
            } else if (op_str == "DEL") {
                op.type = AofOpType::DEL;
            } else {
                continue;
            }

            ops.push_back(std::move(op));
        } catch (const std::exception& e) {
            LOG_WARNING() << "Failed to parse AOF line: " << e.what();
        }
    }

    return ops;
}