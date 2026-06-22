#include "oink_judge/utils/grpc/auth_metadata_plugin.h"

namespace oink_judge::utils::grpc {

AuthMetadataPlugin::AuthMetadataPlugin(std::string token) : token_(std::move(token)) {}

auto AuthMetadataPlugin::GetMetadata(::grpc::string_ref service_url, ::grpc::string_ref method_name,
                                     const ::grpc::AuthContext& channel_auth_context,
                                     std::multimap<::grpc::string, ::grpc::string>* metadata) -> ::grpc::Status {
    metadata->insert({"authorization", token_});
    return ::grpc::Status::OK;
}

} // namespace oink_judge::utils::grpc
