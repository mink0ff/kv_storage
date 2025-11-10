#pragma once
#include <unordered_map>
#include <string>
#include <userver/engine/shared_mutex.hpp>
#include <optional>

#include "persistence/aof_logger.hpp"

class Partition {
    mutable userver::engine::SharedMutex mutex_;
    std::unordered_map<std::string, std::string> kv_;
    uint64_t last_update_operation_idx_{0};

    public:
    // Получить значение по ключу
    std::optional<std::string> Get(const std::string& key) const;

    // Установить значение по ключу
    bool Set(const std::string& key, const std::string& value, const uint64_t op_idx);

    // Удалить ключ
    bool Del(const std::string& key, const uint64_t op_idx);

    // Установить значение без логирования
    void SetNoLog(const std::string& key, const std::string& value, const uint64_t op_idx);

    // Удалить ключ без логирования
    void DelNoLog(const std::string& key, const uint64_t op_idx);

    // Сохранить снапшот в файл
    void Snapshot(const std::string& filename) const;

    // Восстановить партицию из снапшота
    void Recover(const std::string& filename);

    // Применить операцию AOF
    void ApplyOp(const AofOp& op);
};
