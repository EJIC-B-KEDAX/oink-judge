#pragma once
#include "enable_get_test_by_name.hpp"

namespace oink_judge::services::test_node {

class enable_get_test_by_name_impl : public enable_get_test_by_name {
public:
    enable_get_test_by_name_impl();

    std::shared_ptr<Test> get_test_by_name(const std::string &name) override;

protected:
    void add_test(const std::shared_ptr<Test> &test);
    
private:
    std::map<std::string, std::shared_ptr<Test>> _tests;
};

} // namespace oink_judge::services::test_node
