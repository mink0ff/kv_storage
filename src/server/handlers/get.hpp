#include <userver/server/handlers/http_handler_json_base.hpp>
#include "../components/storage_component.hpp"

namespace myservice::handlers {

class Get final : public userver::server::handlers::HttpHandlerJsonBase {
public:
    static constexpr std::string_view kName = "handler-get";
    using HttpHandlerJsonBase::HttpHandlerJsonBase;

    Get(const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context);

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& request_json,
        userver::server::request::RequestContext&) const override;

private:
    std::shared_ptr<Storage> storage_component_;
    
    };

}  // namespace myservice::handlers
