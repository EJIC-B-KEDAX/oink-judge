#pragma once
#include <boost/asio/ssl.hpp>
#include <string>

namespace oink_judge::socket {

class BoostSSLContext {
  public:
    BoostSSLContext(const BoostSSLContext&) = delete;
    auto operator=(const BoostSSLContext&) -> BoostSSLContext& = delete;
    BoostSSLContext(BoostSSLContext&&) = delete;
    auto operator=(BoostSSLContext&&) -> BoostSSLContext& = delete;
    ~BoostSSLContext();

    static auto server() -> boost::asio::ssl::context&;
    static auto client() -> boost::asio::ssl::context&;

  private:
    BoostSSLContext() = default;
    static auto setupSSLContext(boost::asio::ssl::context& context, const std::string& cert_path, const std::string& key_path,
                                const std::string& dh_path) -> void;
};

} // namespace oink_judge::socket
