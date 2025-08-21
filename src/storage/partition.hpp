#pragma once
#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <optional>

#include "utils/utils.hpp"
#include "persistence/aof_logger.hpp"

class Partition {
    mutable std::shared_mutex mutex_; 
    std::unordered_map<std::string, std::string> kv_;

    public:
    // Получить значение по ключу
    std::optional<std::string> Get(const std::string& key) const;

    // Установить значение по ключу
    bool Set(const std::string& key, const std::string& value);

    // Удалить ключ
    bool Del(const std::string& key);

    // Установить значение без логирования
    void SetNoLog(const std::string& key, const std::string& value);
    void DelNoLog(const std::string& key);

    // Сохранить снапшот в файл
    void Snapshot(const std::string& filename) const;

    // Восстановить партицию из снапшота
    void Recover(const std::string& filename);

    // Применить операцию AOF
    void ApplyOp(const AofOp& op);
};
