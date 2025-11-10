#include "utils/utils.hpp"


std::string GetTimestamp() {
    using clock = std::chrono::system_clock;
    auto now = clock::now();
    auto t = clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string TimePointToIsoString(const std::chrono::system_clock::time_point& tp) {
    using clock = std::chrono::system_clock;
    std::time_t t = clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&t);  
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::chrono::system_clock::time_point ParseTimestampIso8601Z(const std::string& s) {
    std::tm tm{};
    std::istringstream iss(s);
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (iss.fail()) {
        throw std::runtime_error("Bad ISO8601 timestamp: " + s);
    }
#if defined(_WIN32)
    time_t tt = _mkgmtime(&tm);
#else
    time_t tt = timegm(&tm);
    if (tt == -1) {
        throw std::runtime_error("timegm failed for: " + s);
    }
#endif
    return std::chrono::system_clock::from_time_t(tt);
}