from collections.abc import AsyncIterator
from contextlib import asynccontextmanager

from oink_judge.pybind11_awaitable_support import AwaitableBridge

_bridge = AwaitableBridge()


@asynccontextmanager
async def awaitable_bridge_lifespan() -> AsyncIterator[None]:
    async with _bridge:
        yield
