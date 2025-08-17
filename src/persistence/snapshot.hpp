#include "storage/storage.hpp"

void Storage::Snapshot() {
    for (auto& shard : shards_) {
        shard->Snapshot("snapshot_" + std::to_string(shard_id) + ".rdb");
    }
}
