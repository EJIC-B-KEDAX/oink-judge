#pragma once

#include <boost/asio/ssl.hpp>
#include <string>

namespace oink_judge::socket {

class BoostSSLContext {
public:
    static boost::asio::ssl::context& instance();

    BoostSSLContext(const BoostSSLContext&) = delete;
    BoostSSLContext& operator=(const BoostSSLContext&) = delete;
    
private:
    BoostSSLContext() = default;
    static void _setup_ssl_context(boost::asio::ssl::context& context, 
                          const std::string& cert_path,
                          const std::string& key_path,
                          const std::string& dh_path);
};

} // namespace oink_judge::socket
