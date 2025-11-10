#pragma once

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>


std::string GetTimestamp();
std::string TimePointToIsoString(const std::chrono::system_clock::time_point& tp);
std::chrono::system_clock::time_point ParseTimestampIso8601Z(const std::string& s);