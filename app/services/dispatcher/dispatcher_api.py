import grpc.aio
from oink_judge.pybind11_logger import log_error, log_info

from app.config import require_has_value
from app.services.dispatcher.dispatcher_service_stub import (
    DispatcherServiceStub,
    get_dispatcher_stub,
)

_stub: DispatcherServiceStub | None = None


def _get_stub() -> DispatcherServiceStub:
    global _stub
    if _stub is None:
        _stub = require_has_value(get_dispatcher_stub())
    return _stub


async def handle_submission(submission_id: str) -> bool:
    try:
        await _get_stub().handle_submission(submission_id)
        log_info("dispatcher_api", f"Handled submission {submission_id}")
        return True
    except grpc.aio.AioRpcError as e:
        log_error("dispatcher_api", f"Failed to handle submission {submission_id}: {e}")
        return False
