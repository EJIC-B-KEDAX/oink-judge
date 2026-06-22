#include "oink_judge/utils/grpc/auth_server_interceptor.h"

#include <oink_judge/logger/logger.h>

namespace oink_judge::utils::grpc {

AuthServerInterceptor::AuthServerInterceptor(std::string expected_token) : expected_token_(std::move(expected_token)) {}

auto AuthServerInterceptor::Intercept(::grpc::experimental::InterceptorBatchMethods* methods) -> void {
    if (methods->QueryInterceptionHookPoint(::grpc::experimental::InterceptionHookPoints::POST_RECV_INITIAL_METADATA)) {
        const auto& client_metadata = methods->GetRecvInitialMetadata();
        auto auth_header = client_metadata->find("authorization");
        if (auth_header == client_metadata->end() || auth_header->second != expected_token_) {
            logger::logWarning("auth_server_interceptor", "Invalid or missing authorization token");
            methods->FailHijackedRecvMessage();
            methods->ModifySendStatus(
                ::grpc::Status(::grpc::StatusCode::UNAUTHENTICATED, "Invalid or missing authorization token"));
            methods->Proceed();
            return;
        }
    }
    methods->Proceed();
}

AuthInterceptorFactory::AuthInterceptorFactory(std::string expected_token) : expected_token_(std::move(expected_token)) {}

auto AuthInterceptorFactory::CreateServerInterceptor(::grpc::experimental::ServerRpcInfo* /*info*/)
    -> ::grpc::experimental::Interceptor* {
    return new AuthServerInterceptor(expected_token_); // NOLINT
}

} // namespace oink_judge::utils::grpc
