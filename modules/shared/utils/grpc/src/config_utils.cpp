#include "oink_judge/utils/grpc/config_utils.h"

#include "oink_judge/utils/grpc/factories.hpp"

#include <oink_judge/config/config.h>

namespace oink_judge::utils::grpc {

auto getServerInterceptorTypes() -> std::vector<std::string> {
    const auto& config_data = config::Config::config();

    if (!config::checkObjectIsArray(config_data, {"grpc", "server_interceptors"})) {
        return {};
    }
    std::vector<std::string> interceptor_types;
    for (const auto& interceptor_type : config_data["grpc"]["server_interceptors"]) {
        if (!interceptor_type.is_string()) {
            throw std::runtime_error("Invalid interceptor type: expected string");
        }
        interceptor_types.push_back(interceptor_type.get<std::string>());
    }
    return interceptor_types;
}

auto getInterceptorsForServer() -> std::vector<std::unique_ptr<::grpc::experimental::ServerInterceptorFactoryInterface>> {
    std::vector<std::unique_ptr<::grpc::experimental::ServerInterceptorFactoryInterface>> interceptors;
    for (const auto& interceptor_type : getServerInterceptorTypes()) {
        interceptors.push_back(utils::grpc::ServerInterceptorFactoriesFactory::instance().create(interceptor_type));
    }
    return interceptors;
}

auto getMyEndpoint() -> std::optional<std::string> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsString(config_data, {"grpc", "my_endpoint"})) {
        return std::nullopt;
    }

    return config_data["grpc"]["my_endpoint"].get<std::string>();
}

auto getServerCredentialsType() -> std::optional<std::string> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsString(config_data, {"grpc", "server_credentials"})) {
        return std::nullopt;
    }
    return config_data["grpc"]["server_credentials"].get<std::string>();
}

auto getServerCredentials() -> std::optional<std::shared_ptr<::grpc::ServerCredentials>> {
    auto credentials_type = getServerCredentialsType();
    if (!credentials_type) {
        return std::nullopt;
    }
    auto credentials = utils::grpc::ServerCredentialFactory::instance().create(*credentials_type);
    if (!credentials) {
        return std::nullopt;
    }
    return credentials;
}

auto getServerChannelArguments() -> std::optional<std::vector<std::pair<std::string, ChannelArgument>>> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsString(config_data, {"grpc", "server_channel_configuration"})) {
        return std::nullopt;
    }
    return getChannelArgumentsList(config_data["grpc"]["server_channel_configuration"].get<std::string>());
}

auto getChannelArgumentsList(const std::string& configuration_name)
    -> std::optional<std::vector<std::pair<std::string, ChannelArgument>>> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsObject(config_data, {"grpc", "channel_arguments_configurations", configuration_name})) {
        return std::nullopt;
    }
    std::vector<std::pair<std::string, ChannelArgument>> arguments;
    for (const auto& [key, value] : config_data["grpc"]["channel_arguments_configurations"][configuration_name].items()) {
        if (value.is_string()) {
            arguments.emplace_back(key, value.get<std::string>());
        } else if (value.is_number()) {
            arguments.emplace_back(key, value.get<int>());
        } else {
            throw std::runtime_error("Invalid argument type: expected string or number");
        }
    }
    return arguments;
}

auto applyServerChannelArguments(::grpc::ServerBuilder& builder) -> void {
    auto arguments_list = getServerChannelArguments();
    if (!arguments_list) {
        throw std::runtime_error("Failed to get server channel arguments list");
    }
    for (const auto& [key, value] : arguments_list.value()) {
        if (std::holds_alternative<std::string>(value)) {
            builder.AddChannelArgument(key, std::get<std::string>(value));
        } else if (std::holds_alternative<int>(value)) {
            builder.AddChannelArgument(key, std::get<int>(value));
        } else {
            throw std::runtime_error("Invalid argument type while applying server channel arguments: expected string or number");
        }
    }
}

auto getChannelArguments(const std::string& configuration_name) -> std::optional<::grpc::ChannelArguments> {
    auto arguments_list = getChannelArgumentsList(configuration_name);
    if (!arguments_list) {
        return std::nullopt;
    }
    ::grpc::ChannelArguments arguments;
    for (const auto& [key, value] : arguments_list.value()) {
        if (std::holds_alternative<std::string>(value)) {
            arguments.SetString(key, std::get<std::string>(value));
        } else if (std::holds_alternative<int>(value)) {
            arguments.SetInt(key, std::get<int>(value));
        } else {
            return std::nullopt;
        }
    }
    return arguments;
}

} // namespace oink_judge::utils::grpc