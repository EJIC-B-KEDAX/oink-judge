#pragma once
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/interceptor.h>

namespace oink_judge::utils::grpc {

class AuthServerInterceptor : public ::grpc::experimental::Interceptor {
  public:
    explicit AuthServerInterceptor(std::string expected_token);
    void Intercept(::grpc::experimental::InterceptorBatchMethods* methods) override;

  private:
    std::string expected_token_;
};

class AuthInterceptorFactory : public ::grpc::experimental::ServerInterceptorFactoryInterface {
  public:
    explicit AuthInterceptorFactory(std::string expected_token);
    auto CreateServerInterceptor(::grpc::experimental::ServerRpcInfo* info) -> ::grpc::experimental::Interceptor* override;

    constexpr static auto REGISTERED_NAME = "auth_req";

  private:
    std::string expected_token_;
};

} // namespace oink_judge::utils::grpc
