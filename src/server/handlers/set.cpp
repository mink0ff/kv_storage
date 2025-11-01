#include "set.hpp"
#include "../handlers_make_response/handlers_make_response.hpp"


namespace myservice::handlers {
    Set::Set(const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context):
        HttpHandlerJsonBase(config, context),
        storage_component_(context.FindComponent<myservice::components::StorageComponent>().GetStorage())
    {}

    userver::formats::json::Value Set::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& request_json,
        userver::server::request::RequestContext&) const 
    {
        const auto key = request_json["key"].As<std::string>({});
        const auto value = request_json["value"].As<std::string>({});

        if (key.empty() || value.empty()) {
            userver::formats::json::ValueBuilder response;
            response["success"] = false;
            response["error"] = "Missing 'key' or 'value'";
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kBadRequest);
            return response.ExtractValue();
        }

        auto response_builder = MakeSetKeyResponse(*storage_component_, key, value);
        auto response = response_builder.ExtractValue();

        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
        return response;
    }
}

