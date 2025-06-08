#include <iostream>
#include "config/config.h"

int32_t main() {
    std::cout << oink_judge::config::Config::instance().get_port("auth") << std::endl;
}