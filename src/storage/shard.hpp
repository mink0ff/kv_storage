#include <unordered_map>
#include <shared_mutex>
#include <optional>


class Shard {
    std::unordered_map<std::string, std::string> kv_;
    mutable std::shared_mutex shard_mutex_;
public:
    std::optional<std::string> Get(const std::string& key);
    void Set(const std::string& key, const std::string& value);
    void Del(const std::string& key);
    void Snapshot(const std::string& filename);
};
