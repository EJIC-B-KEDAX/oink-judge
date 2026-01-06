#include "socket/BoostSSLContext.h"
#include "config/Config.h"
#include <stdexcept>
#include <iostream>

namespace oink_judge::socket {

using Config = config::Config;

boost::asio::ssl::context& BoostSSLContext::server() {
    static boost::asio::ssl::context server_context(boost::asio::ssl::context::tlsv12_server);
    static bool initialized = false;


    if (!initialized) {
        std::string certs_dir = Config::config().at("directories").at("certs").get<std::string>();
        _setup_ssl_context(server_context, certs_dir + "/server.crt", certs_dir + "/server.key", certs_dir + "/dh2048.pem");
        initialized = true;
        std::cout << "Server SSL context initialized" << std::endl;
    }
    
    return server_context;
}

boost::asio::ssl::context& BoostSSLContext::client() {
    static boost::asio::ssl::context client_context(boost::asio::ssl::context::tlsv12_client);
    static bool initialized = false;
    
    if (!initialized) {
        std::string certs_dir = Config::config().at("directories").at("certs").get<std::string>();
        client_context.load_verify_file(certs_dir + "/ca.pem");
        client_context.set_verify_mode(boost::asio::ssl::verify_peer);

        initialized = true;
        std::cout << "Client SSL context initialized" << std::endl;
    }
    
    return client_context;
}

void BoostSSLContext::_setup_ssl_context(boost::asio::ssl::context& context,
                                      const std::string& cert_path,
                                      const std::string& key_path,
                                      const std::string& dh_path) {
    context.set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2 |
        boost::asio::ssl::context::no_sslv3 |
        boost::asio::ssl::context::single_dh_use);

    try {
        context.use_certificate_chain_file(cert_path);
        context.use_private_key_file(key_path, boost::asio::ssl::context::pem);
        context.use_tmp_dh_file(dh_path);

        // Set strong cipher list
        SSL_CTX_set_cipher_list(context.native_handle(), 
            "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK");
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to set up SSL context: " + std::string(e.what()));
    }
}

} // namespace oink_judge::socket
