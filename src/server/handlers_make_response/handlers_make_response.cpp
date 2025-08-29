#include "handlers_make_response.hpp"

userver::formats::json::ValueBuilder MakeGetKeyResponse(
    const Storage& storage,
    const std::string& key) 
{
    userver::formats::json::ValueBuilder response(userver::formats::common::Type::kObject);

    auto value_opt = storage.Get(key);
    if (value_opt) {
        response["found"] = true;
        response["key"] = key;
        response["value"] = *value_opt;
    } else {
        response["found"] = false;
        response["key"] = key;
        response["error"] = "Key not found";
    }

    return response;
}

userver::formats::json::ValueBuilder MakeSetKeyResponse(
    Storage& storage,
    const std::string& key, 
    const std::string& value) 
{
    userver::formats::json::ValueBuilder response(userver::formats::common::Type::kObject);

    auto success = storage.Set(key, value);
    if (success) {
        response["result"] = "success";
        response["key"] = key;
        response["value"] = value;
    } else {
        response["result"] = "failure";
        response["key"] = key;
        response["error"] = "Failed to set key-value pair";
    }

    return response;
}

userver::formats::json::ValueBuilder MakeDelKeyResponse(
    Storage& storage,
    const std::string& key) 
{
    userver::formats::json::ValueBuilder response(userver::formats::common::Type::kObject);

    auto success = storage.Del(key);
    if (success) {
        response["result"] = "success";
        response["key"] = key;
    } else {
        response["result"] = "failure";
        response["key"] = key;
        response["error"] = "Key not found";
    }

    return response;
}