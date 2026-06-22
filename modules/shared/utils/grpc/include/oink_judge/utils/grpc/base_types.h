#pragma once

namespace oink_judge::utils::grpc {

auto registerChannelType() -> void;
auto registerCustomChannelType() -> void;
auto registerInsecureCredentialsType() -> void;
auto registerSslServerCredentialsType() -> void;
auto registerSslClientCredentialsType() -> void;
auto registerAuthCredentialsType() -> void;
auto registerAuthServerInterceptorFactoryType() -> void;

} // namespace oink_judge::utils::grpc
