#include <oink_judge/utils/grpc/base_types.h>

extern "C" auto registerTypes() -> void {
    oink_judge::utils::grpc::registerChannelType();
    oink_judge::utils::grpc::registerCustomChannelType();
    oink_judge::utils::grpc::registerInsecureCredentialsType();
    oink_judge::utils::grpc::registerSslServerCredentialsType();
    oink_judge::utils::grpc::registerSslClientCredentialsType();
    oink_judge::utils::grpc::registerAuthCredentialsType();
    oink_judge::utils::grpc::registerAuthServerInterceptorFactoryType();
}
