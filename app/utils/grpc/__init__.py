from oink_judge.pybind11_utils_grpc import (
    get_my_endpoint,
    get_server_credentials_type,
    get_server_interceptor_types,
)

from .base_types import (
    register_auth_credentials_type,
    register_channel_type,
    register_custom_channel_type,
    register_insecure_channel_type,
    register_ssl_client_credentials_type,
)
from .factories import CallCredentialFactory, ChannelCredentialFactory, ChannelFactory

register_channel_type()
register_custom_channel_type()
register_insecure_channel_type()
register_ssl_client_credentials_type()
register_auth_credentials_type()

__all__ = [
    "get_my_endpoint",
    "get_server_credentials_type",
    "get_server_interceptor_types",
    "ChannelFactory",
    "ChannelCredentialFactory",
    "CallCredentialFactory",
]
