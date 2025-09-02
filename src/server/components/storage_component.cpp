#include "storage_component.hpp"
#include <userver/logging/log.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/component_base.hpp>



namespace myservice::components {
    StorageComponent::StorageComponent(const userver::components::ComponentConfig& config,
                                             const userver::components::ComponentContext& context)
        : ComponentBase(config, context),
            storage_(std::make_shared<Storage>(
                config["partitions"].As<int>(16),
                config["aof-path"].As<std::string>("logs/appendonly.aof"),
                config["snapshot-dir"].As<std::string>("snapshots/test_snapshots")))
    {}

    std::shared_ptr<Storage> StorageComponent::GetStorage() { return storage_; }

    userver::yaml_config::Schema StorageComponent::GetStaticConfigSchema() {
        // LOG_INFO() << "StorageComponent::GetStaticConfigSchema CALLED";
        return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: Storage component for KV store
additionalProperties: false
properties:
    fs-task-processor:
        type: string
        description: Task processor for filesystem operations
    partitions:
        type: integer
        description: Number of partitions
        minimum: 1
    aof-path:
        type: string
        description: Path to AOF log file
    snapshot-dir:
        type: string
        description: Directory for snapshots
)");
    }

}  // namespace myservice::components
