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
    {
        auto snapshot_interval = config["snapshot-interval"].As<std::chrono::seconds>(std::chrono::seconds{600});

        periodic_snapshot_.Start(
            "storage-snapshot",
            snapshot_interval,
            [this] {
                try {
                    storage_->Snapshot();
                    LOG_INFO() << "Snapshot done";
                } catch (const std::exception& e) {
                    LOG_ERROR() << "Snapshot failed: " << e.what();
                }
            });
    }

    StorageComponent::~StorageComponent(){
        periodic_snapshot_.Stop();
        try {
            storage_->Snapshot();
            LOG_INFO() << "Final snapshot done";
        } catch (const std::exception& e) {
            LOG_ERROR() << "Final snapshot failed: " << e.what();
        }
    }

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
    snapshot-interval:
        type: string
        description: Interval between automatic snapshots (e.g., '60s')
        defaultDescription: '60s'
)");
    }

}  // namespace myservice::components
