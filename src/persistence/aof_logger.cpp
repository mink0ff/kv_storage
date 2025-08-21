#include "aof_logger.hpp"

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

void AofLogger::Append(const std::string& command) {
    std::lock_guard<std::mutex> lock(mtx_);
    file_ << GetTimestamp() + ' ' + command << "\n";
    file_.flush();  
}

void AofLogger::Flush() {
    std::lock_guard<std::mutex> lock(mtx_);
    file_.flush();
}

void AofLogger::Clear() {
    std::lock_guard<std::mutex> lg(mtx_);

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
        std::istringstream iss(line);

        std::string ts_str, op_str;
        std::size_t pid;
        if (!(iss >> ts_str >> pid >> op_str)) continue;

        AofOp op;
        op.ts = ParseTimestampIso8601Z(ts_str);
        op.partition_id = pid;

        if (op_str == "SET") {
            std::string key, value;
            if (!(iss >> key >> value)) continue;
            op.type = AofOpType::SET;
            op.key = std::move(key);
            op.value = std::move(value);
        } else if (op_str == "DEL") {
            std::string key;
            if (!(iss >> key)) continue;
            op.type = AofOpType::DEL;
            op.key = std::move(key);
        } else {
            continue;
        }
        ops.emplace_back(std::move(op));
    }

    return ops;
}