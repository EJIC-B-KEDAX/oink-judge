#include "oink_judge/socket/boost_ssl_context.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/logger/logger.h>

#include <stdexcept>

namespace oink_judge::socket {

using config::requireHasValue;

namespace fs = std::filesystem;

auto BoostSSLContext::server() -> boost::asio::ssl::context& {
    static boost::asio::ssl::context server_context(boost::asio::ssl::context::tlsv12_server);
    static bool initialized = false;

    if (!initialized) {
        fs::path certs_dir = requireHasValue(config::getDirectoryPath("certs"));
        setupSSLContext(server_context, certs_dir / "server.crt", certs_dir / "server.key", certs_dir / "dh2048.pem");
        initialized = true;
    }

    return server_context;
}

auto BoostSSLContext::client() -> boost::asio::ssl::context& {
    static boost::asio::ssl::context client_context(boost::asio::ssl::context::tlsv12_client);
    static bool initialized = false;

    if (!initialized) {
        fs::path certs_dir = requireHasValue(config::getDirectoryPath("certs"));
        client_context.load_verify_file(certs_dir / "ca.pem");
        client_context.set_verify_mode(boost::asio::ssl::verify_peer);

        initialized = true;
    }

    return client_context;
}

void BoostSSLContext::setupSSLContext(boost::asio::ssl::context& context, const fs::path& cert_path, const fs::path& key_path,
                                      const fs::path& dh_path) {
    context.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 |
                        boost::asio::ssl::context::no_sslv3 | boost::asio::ssl::context::single_dh_use);

    try {
        context.use_certificate_chain_file(cert_path);
        context.use_private_key_file(key_path, boost::asio::ssl::context::pem);
        context.use_tmp_dh_file(dh_path);

        // Set strong cipher list
        SSL_CTX_set_cipher_list(
            context.native_handle(),
            "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:"
            "DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:"
            "ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:"
            "ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-"
            "AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK");
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to set up SSL context: " + std::string(e.what()));
    }
}

} // namespace oink_judge::socket
