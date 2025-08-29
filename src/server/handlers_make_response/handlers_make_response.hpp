#include <userver/formats/json/value_builder.hpp>
#include "../../storage/storage.hpp"

userver::formats::json::ValueBuilder MakeGetKeyResponse(
    const Storage& storage,
    const std::string& key);


userver::formats::json::ValueBuilder MakeSetKeyResponse(
    Storage& storage,
    const std::string& key, 
    const std::string& value);

userver::formats::json::ValueBuilder MakeDelKeyResponse(
    const Storage& storage,
    const std::string& key);