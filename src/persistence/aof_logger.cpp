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


std::string AofLogger::GetTimestamp() {
    using clock = std::chrono::system_clock;
    auto now = clock::now();
    auto t = clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

void AofLogger::Clear() {
    std::lock_guard<std::mutex> lg(mtx_);

    // Закрываем текущий поток
    if (file_.is_open()) {
        file_.close();
    }

    // Перезаписываем файл (truncate)
    file_.open(filename_, std::ios::trunc);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to truncate AOF file: " + filename_);
    }

    // Закрыть и открыть заново в режиме append
    file_.close();
    file_.open(filename_, std::ios::app);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to reopen AOF file: " + filename_);
    }
}