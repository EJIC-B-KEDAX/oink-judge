#pragma once
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>

#include <memory>
#include <optional>
#include <vector>

namespace oink_judge::utils::grpc {

using ChannelArgument = std::variant<std::string, int>;

auto getInterceptorsForServer() -> std::vector<std::unique_ptr<::grpc::experimental::ServerInterceptorFactoryInterface>>;
auto getMyEndpoint() -> std::optional<std::string>;
auto getServerCredentials() -> std::optional<std::shared_ptr<::grpc::ServerCredentials>>;
auto getServerCredentialsType() -> std::optional<std::string>;
auto getServerInterceptorTypes() -> std::vector<std::string>;
auto getServerChannelArguments() -> std::optional<std::vector<std::pair<std::string, ChannelArgument>>>;
auto getChannelArgumentsList(const std::string& configuration_name)
    -> std::optional<std::vector<std::pair<std::string, ChannelArgument>>>;
auto applyServerChannelArguments(::grpc::ServerBuilder& builder) -> void;
auto getChannelArguments(const std::string& configuration_name) -> std::optional<::grpc::ChannelArguments>;

} // namespace oink_judge::utils::grpc
