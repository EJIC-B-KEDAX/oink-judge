import grpc
import grpc.aio
from oink_judge.pybind11_utils_grpc import get_channel_arguments_list

from app.config import get_token_from_credentials
from app.factory import normalize_argument, parse_parameters
from app.utils import load_file

from .auth_metadata_plugin import TokenAuthPlugin
from .factories import CallCredentialFactory, ChannelCredentialFactory, ChannelFactory


def register_channel_type() -> None:
    def channel(params: str) -> grpc.aio.Channel:

        parts = parse_parameters(params, ",")
        if len(parts) < 2:
            raise ValueError(
                "Channel requires at least 2 parameters: endpoint, channel_credentials, [call_credentials...]"
            )
        endpoint = normalize_argument(parts[0], True)
        channel_credentials = ChannelCredentialFactory.instance().create(parts[1])
        for i in range(2, len(parts)):
            call_credentials = CallCredentialFactory.instance().create(parts[i])
            if call_credentials is not None:
                channel_credentials = grpc.composite_channel_credentials(
                    channel_credentials, call_credentials
                )
            else:
                raise ValueError(
                    f"Failed to create call credentials for parameter: {parts[i]}"
                )
        return grpc.aio.secure_channel(endpoint, channel_credentials)

    ChannelFactory.instance().register_type("channel", channel)


def register_custom_channel_type() -> None:
    def custom_channel(params: str) -> grpc.aio.Channel:
        parts = parse_parameters(params, ",")
        if len(parts) < 3:
            raise ValueError(
                "Custom channel requires at least 3 parameters: endpoint, arguments_configuration, channel_credentials, [call_credentials...]"
            )
        endpoint = normalize_argument(parts[0], True)
        arguments_configuration = normalize_argument(parts[1], True)
        arguments_list = get_channel_arguments_list(arguments_configuration)
        if arguments_list is None:
            raise ValueError(
                f"Failed to get channel arguments list for configuration: {arguments_configuration}"
            )

        channel_credentials = ChannelCredentialFactory.instance().create(parts[2])
        for i in range(3, len(parts)):
            call_credentials = CallCredentialFactory.instance().create(parts[i])
            if call_credentials is not None:
                channel_credentials = grpc.composite_channel_credentials(
                    channel_credentials, call_credentials
                )
            else:
                raise ValueError(
                    f"Failed to create call credentials for parameter: {parts[i]}"
                )
        return grpc.aio.secure_channel(endpoint, channel_credentials, arguments_list)

    ChannelFactory.instance().register_type("custom_channel", custom_channel)


def register_insecure_channel_type() -> None:
    def channel(params: str) -> grpc.aio.Channel:

        parts = parse_parameters(params, ",")
        if len(parts) != 1:
            raise ValueError("Insecure channel requires exactly 1 parameter: endpoint")
        endpoint = normalize_argument(parts[0], True)
        return grpc.aio.insecure_channel(endpoint)

    ChannelFactory.instance().register_type("insecure", channel)


def register_ssl_client_credentials_type() -> None:
    def ssl_client_credentials(params: str) -> grpc.ChannelCredentials:
        parts = parse_parameters(params, ",")
        if len(parts) != 1:
            raise ValueError(
                "SSL client credentials require exactly 1 parameter: path to pem root certs"
            )
        path_to_pem_root_certs = parts[0]
        pem_root_certs = load_file(path_to_pem_root_certs)
        return grpc.ssl_channel_credentials(pem_root_certs.encode())

    ChannelCredentialFactory.instance().register_type(
        "ssl_client", ssl_client_credentials
    )


def register_auth_credentials_type() -> None:
    def auth_credentials(params: str) -> grpc.CallCredentials:
        parts = parse_parameters(params, ",")
        if len(parts) != 1:
            raise ValueError(
                "Auth credentials require exactly 1 parameter: path to token"
            )
        path_to_token = parts[0]
        token = get_token_from_credentials(path_to_token)
        if token is None:
            raise ValueError(
                f"Failed to retrieve token from credentials for parameter: {path_to_token}"
            )
        auth_plugin = TokenAuthPlugin(token)
        return grpc.metadata_call_credentials(auth_plugin)

    CallCredentialFactory.instance().register_type("auth", auth_credentials)
