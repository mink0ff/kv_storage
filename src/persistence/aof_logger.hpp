#pragma once
#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

class AofLogger {
public:
    explicit AofLogger(const std::string& filename);
    ~AofLogger();

    void Append(const std::string& command); 
    void Flush();
    std::string GetTimestamp();


private:
    std::ofstream file_;
    std::mutex mtx_;
};
