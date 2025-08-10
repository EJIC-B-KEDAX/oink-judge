#pragma once

#include <boost/asio.hpp>

namespace oink_judge::socket {

class BoostIOContext {
public:
    static boost::asio::io_context& instance();

    BoostIOContext(const BoostIOContext&) = delete;
    BoostIOContext& operator=(const BoostIOContext&) = delete;
    
private:
    BoostIOContext() = default;
};

} // namespace oink_judge::socket
