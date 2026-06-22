#include "oink_judge/content_service/client/config_utils.h"

#include "oink_judge/content_service/client/content_service_stub.h"

#include <oink_judge/config/config.h>

#include <string>

namespace oink_judge::content_service {

auto getContentStorageStubType() -> std::optional<std::string> {
    const auto& config = config::Config::config();
    if (!config::checkObjectIsString(config, {"content_storage", "stub_type"})) {
        return std::nullopt;
    }
    return config["content_storage"]["stub_type"].get<std::string>();
}

auto getContentStorageStub() -> std::optional<std::unique_ptr<ContentServiceStub>> {
    auto stub_type_opt = getContentStorageStubType();
    if (!stub_type_opt.has_value()) {
        return std::nullopt;
    }
    return ContentServiceStubFactory::instance().create(*stub_type_opt);
}

} // namespace oink_judge::content_service
