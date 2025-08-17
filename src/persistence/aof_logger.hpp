#include <mutex>
#include <fstream>



class AOFLogger {
    std::mutex file_mutex_;
    std::ofstream out_;
public:
    AOFLogger(const std::string& filename);
    void Append(const std::string& cmd);
};
