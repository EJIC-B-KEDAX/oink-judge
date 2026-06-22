#pragma once
#include <nlohmann/json.hpp>

#include <chrono>

namespace oink_judge::content_service {

using nlohmann::json;

class ContentScanner {
  public:
    ContentScanner(std::string content_type, std::string content_id);

    [[nodiscard]] auto scanContent() -> json;

  private:
    std::string content_type_;
    std::string content_id_;

    std::chrono::duration<double> full_rescan_interval_;
    std::chrono::milliseconds last_full_rescan_;

    [[nodiscard]] auto fullContentScan() -> json;
    [[nodiscard]] auto scanContentWithCache() -> json;
};

} // namespace oink_judge::content_service
