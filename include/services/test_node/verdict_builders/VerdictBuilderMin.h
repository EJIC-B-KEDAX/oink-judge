#pragma once
#include "services/test_node/VerdictBuilder.hpp"

namespace oink_judge::services::test_node {

class VerdictBuilderMin : public VerdictBuilder {
public:
    VerdictBuilderMin(const std::string &test_name);

    void clear() override;
    void add_verdict(std::shared_ptr<DefaultVerdict> verdict) override;
    bool can_score_change() const override;
    std::shared_ptr<DefaultVerdict> finalize() override;

    constexpr static auto REGISTERED_NAME = "min";

private:
    std::string _test_name;
    std::shared_ptr<DefaultVerdict> _current_verdict;
};

} // namespace oink_judge::services::test_node
