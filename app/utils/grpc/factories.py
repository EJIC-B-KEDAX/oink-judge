import grpc
import grpc.aio

from app.factory import ParameterizedTypeFactory


class ChannelFactory(ParameterizedTypeFactory[grpc.aio.Channel]): ...


class ChannelCredentialFactory(ParameterizedTypeFactory[grpc.ChannelCredentials]): ...


class CallCredentialFactory(ParameterizedTypeFactory[grpc.CallCredentials]): ...
