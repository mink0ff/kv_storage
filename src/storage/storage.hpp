#include <vector>
#include <memory>

#include "persistence/aof_logger.hpp"
#include "storage/shard.hpp"

class Storage {
    std::vector<std::unique_ptr<Shard>> shards_;
    size_t num_shards_;
    AOFLogger aof_logger_;  // отдельный поток для записи AOF
public:
    Storage(size_t num_shards);
    std::optional<std::string> Get(const std::string& key);
    void Set(const std::string& key, const std::string& value);
    void Del(const std::string& key);
    void Snapshot();
private:
    Shard& SelectShard(const std::string& key);
};
