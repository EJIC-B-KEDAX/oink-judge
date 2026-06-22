#include "oink_judge/utils/grpc/base_types.h"

#include "oink_judge/utils/grpc/auth_metadata_plugin.h"
#include "oink_judge/utils/grpc/auth_server_interceptor.h"
#include "oink_judge/utils/grpc/config_utils.h"
#include "oink_judge/utils/grpc/factories.hpp"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/factory/parameterized_type_factory.hpp>
#include <oink_judge/utils/filesystem.h>

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>

#include <string>
#include <vector>

namespace oink_judge::utils::grpc {

auto registerChannelType() -> void {
    ChannelFactory::instance().registerType("channel", [](const std::string& params) -> std::shared_ptr<::grpc::Channel> {
        std::vector<std::string> parts = factory::parseParameters(params, ",");
        if (parts.size() < 2) {
            throw std::invalid_argument(
                "Insecure channel requires at least 2 parameters: endpoint, channel_credentials, [call_credentials...]");
        }
        std::string endpoint = factory::normalizeArgument(parts[0], true);
        auto current_credentials = ChannelCredentialFactory::instance().create(parts[1]);
        for (size_t i = 2; i < parts.size(); ++i) {
            auto additional_credentials = CallCredentialFactory::instance().create(parts[i]);
            if (additional_credentials) {
                current_credentials = ::grpc::CompositeChannelCredentials(current_credentials, additional_credentials);
            } else {
                throw std::invalid_argument("Failed to create credentials for parameter: " + parts[i]);
            }
        }
        return ::grpc::CreateChannel(endpoint, current_credentials);
    });
}

auto registerCustomChannelType() -> void {
    ChannelFactory::instance().registerType("custom_channel", [](const std::string& params) -> std::shared_ptr<::grpc::Channel> {
        std::vector<std::string> parts = factory::parseParameters(params, ",");
        if (parts.size() < 3) {
            throw std::invalid_argument("Custom channel requires at least 3 parameters: endpoint, "
                                        "arguments_configuration, channel_credentials, [call_credentials...]");
        }
        std::string endpoint = factory::normalizeArgument(parts[0], true);
        std::string arguments_configuration = factory::normalizeArgument(parts[1], true);
        auto arguments = getChannelArguments(arguments_configuration);
        if (!arguments) {
            throw std::invalid_argument("Failed to get channel arguments for configuration: " + arguments_configuration);
        }
        auto current_credentials = ChannelCredentialFactory::instance().create(parts[2]);
        for (size_t i = 3; i < parts.size(); ++i) {
            auto additional_credentials = CallCredentialFactory::instance().create(parts[i]);
            if (additional_credentials) {
                current_credentials = ::grpc::CompositeChannelCredentials(current_credentials, additional_credentials);
            } else {
                throw std::invalid_argument("Failed to create credentials for parameter: " + parts[i]);
            }
        }
        auto channel = ::grpc::CreateCustomChannel(endpoint, current_credentials, arguments.value());
        return channel;
    });
}

auto registerInsecureCredentialsType() -> void {
    ChannelCredentialFactory::instance().registerType(
        "insecure", [](const std::string& /*params*/) -> std::shared_ptr<::grpc::ChannelCredentials> {
            return ::grpc::InsecureChannelCredentials();
        });
}

auto registerSslServerCredentialsType() -> void {
    ServerCredentialFactory::instance().registerType(
        "ssl_server", [](const std::string& params) -> std::shared_ptr<::grpc::ServerCredentials> {
            std::vector<std::string> parts = factory::parseParameters(params, ",");
            if (parts.size() != 2) {
                throw std::invalid_argument("SSL server credentials require exactly 2 parameters: pem_root_certs, private_key");
            }
            ::grpc::SslServerCredentialsOptions ssl_opts;
            ssl_opts.pem_key_cert_pairs.push_back({
                .private_key = filesystem::loadFile(factory::normalizeArgument(parts[1], true)), // private_key
                .cert_chain = filesystem::loadFile(factory::normalizeArgument(parts[0], true)),  // pem_root_certs
            });

            return ::grpc::SslServerCredentials(ssl_opts);
        });
}

auto registerSslClientCredentialsType() -> void {
    ChannelCredentialFactory::instance().registerType(
        "ssl_client", [](const std::string& params) -> std::shared_ptr<::grpc::ChannelCredentials> {
            std::vector<std::string> parts = factory::parseParameters(params, ",");
            if (parts.size() != 1) {
                throw std::invalid_argument("SSL client credentials require exactly 1 parameter: pem_root_certs");
            }
            ::grpc::SslCredentialsOptions ssl_opts;
            ssl_opts.pem_root_certs = filesystem::loadFile(factory::normalizeArgument(parts[0], true));
            return ::grpc::SslCredentials(ssl_opts);
        });
}

auto registerAuthCredentialsType() -> void {
    CallCredentialFactory::instance().registerType(
        AuthMetadataPlugin::REGISTERED_NAME, [](const std::string& params) -> std::shared_ptr<::grpc::CallCredentials> {
            std::vector<std::string> parts = factory::parseParameters(params, ",");
            if (parts.size() != 1) {
                throw std::invalid_argument("Auth credentials require exactly 1 parameter: auth_token");
            }
            std::string path_to_token = factory::normalizeArgument(parts[0], true);
            std::optional<std::string> token_opt = config::getTokenFromCredentials(path_to_token);
            if (token_opt == std::nullopt) {
                throw std::invalid_argument("Failed to retrieve token from credentials for parameter: " + path_to_token);
            }
            std::string token = token_opt.value();
            return ::grpc::MetadataCredentialsFromPlugin(std::make_unique<AuthMetadataPlugin>(token));
        });
}

auto registerAuthServerInterceptorFactoryType() -> void {
    ServerInterceptorFactoriesFactory::instance().registerType(
        AuthInterceptorFactory::REGISTERED_NAME,
        [](const std::string& params) -> std::unique_ptr<::grpc::experimental::ServerInterceptorFactoryInterface> {
            std::vector<std::string> parts = factory::parseParameters(params, ",");
            if (parts.size() != 1) {
                throw std::invalid_argument("Auth server interceptor factory requires exactly 1 parameter: expected_token");
            }
            std::string path_to_token = factory::normalizeArgument(parts[0], true);
            std::optional<std::string> token_opt = config::getTokenFromCredentials(path_to_token);
            if (token_opt == std::nullopt) {
                throw std::invalid_argument("Failed to retrieve token from credentials for parameter: " + path_to_token);
            }
            std::string expected_token = token_opt.value();
            return std::make_unique<AuthInterceptorFactory>(expected_token);
        });
}

} // namespace oink_judge::utils::grpc
