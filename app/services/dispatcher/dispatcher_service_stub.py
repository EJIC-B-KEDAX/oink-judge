from typing import Optional

import grpc.aio
from oink_judge import dispatcher_pb2 as pb2
from oink_judge import dispatcher_pb2_grpc as pb2_grpc
from oink_judge.pybind11_dispatcher import get_dispatcher_stub_type

import app.utils.grpc  # noqa: F401 — registers channel/credential factory types
from app.factory import ParameterizedTypeFactory, normalize_argument, parse_parameters
from app.utils.grpc import ChannelFactory


class DispatcherServiceStubFactory(ParameterizedTypeFactory["DispatcherServiceStub"]): ...


def get_dispatcher_stub() -> Optional["DispatcherServiceStub"]:
    stub_type = get_dispatcher_stub_type()
    if stub_type is None:
        return None
    return DispatcherServiceStubFactory.instance().create(stub_type)


def register_dispatcher_service_channel_stub_type() -> None:
    def channel_stub(params: str) -> "DispatcherServiceStub":
        parts = parse_parameters(params, ",")
        if len(parts) != 1:
            raise ValueError(
                "Expected exactly one parameter for dispatcher_service_stub: channel type"
            )
        channel_spec = normalize_argument(parts[0], True)
        channel = ChannelFactory.instance().create(channel_spec)
        return DispatcherServiceStub(channel)

    DispatcherServiceStubFactory.instance().register_type(
        "dispatcher_service_stub", channel_stub
    )


register_dispatcher_service_channel_stub_type()


class DispatcherServiceStub:
    _stub: pb2_grpc.DispatcherServiceStub

    def __init__(self, channel: grpc.aio.Channel) -> None:
        self._stub = pb2_grpc.DispatcherServiceStub(channel)

    async def handle_submission(self, submission_id: str) -> None:
        request = pb2.HandleSubmissionRequest(submission_id=submission_id)
        await self._stub.HandleSubmission(request)
