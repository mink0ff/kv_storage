#pragma once

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/loggable_component_base.hpp>
#include <userver/yaml_config/schema.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "storage/storage.hpp"

namespace myservice::components {

class StorageComponent final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "storage";

    StorageComponent(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

    std::shared_ptr<Storage> GetStorage();

    static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    std::shared_ptr<Storage> storage_;
};

}  // namespace myservice::components
