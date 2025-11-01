#include "get.hpp"
#include "../handlers_make_response/handlers_make_response.hpp"
#include <userver/server/http/http_request.hpp>

namespace myservice::handlers {

    Get::Get(const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context):
        HttpHandlerJsonBase(config, context),
        storage_component_(context.FindComponent<myservice::components::StorageComponent>().GetStorage())
    {}

    userver::formats::json::Value Get::HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& _json,
        userver::server::request::RequestContext&) const  
    {
        const auto key = request.GetArg("key");
        
        auto response_builder = MakeGetKeyResponse(*storage_component_, key);
        auto response = response_builder.ExtractValue();

        if (!response["found"].As<bool>()) {
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kNotFound);
        } else {
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kOk);
        }
        return response;
    }

}  // namespace myservice::handlers
