#pragma once
#include "oink_judge/content_service/client/content_service_stub.h"

#include <memory>
#include <optional>
#include <string>

namespace oink_judge::content_service {

auto getContentStorageStubType() -> std::optional<std::string>;
auto getContentStorageStub() -> std::optional<std::unique_ptr<ContentServiceStub>>;

} // namespace oink_judge::content_service
