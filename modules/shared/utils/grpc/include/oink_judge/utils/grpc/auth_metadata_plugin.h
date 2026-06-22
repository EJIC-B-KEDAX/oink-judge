#pragma once
#include <grpcpp/grpcpp.h>

namespace oink_judge::utils::grpc {

class AuthMetadataPlugin : public ::grpc::MetadataCredentialsPlugin {
  public:
    explicit AuthMetadataPlugin(std::string token);

    auto GetMetadata(::grpc::string_ref service_url, ::grpc::string_ref method_name,
                     const ::grpc::AuthContext& channel_auth_context, std::multimap<::grpc::string, ::grpc::string>* metadata)
        -> ::grpc::Status override;

    constexpr static auto REGISTERED_NAME = "auth";

  private:
    std::string token_;
};

} // namespace oink_judge::utils::grpc
