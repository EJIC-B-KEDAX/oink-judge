#pragma once

#include <oink_judge/factory/parameterized_type_factory.hpp>

#include <grpcpp/grpcpp.h>

namespace oink_judge::utils::grpc {

using ChannelFactory = factory::ParameterizedTypeFactory<std::shared_ptr<::grpc::Channel>>;

using ServerCredentialFactory = factory::ParameterizedTypeFactory<std::shared_ptr<::grpc::ServerCredentials>>;
using ChannelCredentialFactory = factory::ParameterizedTypeFactory<std::shared_ptr<::grpc::ChannelCredentials>>;
using CallCredentialFactory = factory::ParameterizedTypeFactory<std::shared_ptr<::grpc::CallCredentials>>;

using ServerInterceptorFactoriesFactory =
    factory::ParameterizedTypeFactory<std::unique_ptr<::grpc::experimental::ServerInterceptorFactoryInterface>>;
using ClientInterceptorFactoriesFactory =
    factory::ParameterizedTypeFactory<std::unique_ptr<::grpc::experimental::ClientInterceptorFactoryInterface>>;

} // namespace oink_judge::utils::grpc
