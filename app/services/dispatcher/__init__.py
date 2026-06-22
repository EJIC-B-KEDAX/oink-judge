from oink_judge.pybind11_dispatcher import get_dispatcher_stub_type

from .dispatcher_api import handle_submission
from .dispatcher_service_stub import (
    DispatcherServiceStub,
    DispatcherServiceStubFactory,
    get_dispatcher_stub,
)

__all__ = [
    "DispatcherServiceStub",
    "DispatcherServiceStubFactory",
    "get_dispatcher_stub",
    "get_dispatcher_stub_type",
    "handle_submission",
]
