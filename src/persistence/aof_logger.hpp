#pragma once
#include <fstream>
#include <userver/engine/mutex.hpp>
#include <string>
#include <vector>


enum class AofOpType { SET, DEL };

struct AofOp {
    std::chrono::system_clock::time_point ts;
    uint64_t operation_idx{};
    std::size_t partition_id{};
    AofOpType type{};
    std::string key;
    std::string value;
};

class AofLogger {
public:
    explicit AofLogger(const std::string& filename);
    ~AofLogger();

    void Append(const AofOp& op); 
    void Flush();

    void Clear();

    std::vector<AofOp> ReadAll() const;
    std::vector<AofOp> ReadFile(const std::string& path) const;


private:
    std::ofstream file_;
    userver::engine::Mutex mutex_;
    std::string filename_;
};
