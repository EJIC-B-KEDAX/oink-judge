"""Python integration tests for database awaitables (no live DB required)."""

import asyncio

from oink_judge.pybind11_awaitable_support import AwaitableBridge
from oink_judge.pybind11_database import TableSubmissions


def test_load_submissions_auto_initializes_without_config():
    async def run() -> None:
        async with AwaitableBridge():
            submissions = await TableSubmissions.instance().load_submissions_by_user_and_problem("user", "problem")
            assert submissions is None

    asyncio.run(run())
