#include <iostream>
#include "config/Config.h"

int32_t main() {
    std::cout << oink_judge::config::Config::instance().get_port("auth") << std::endl;
}