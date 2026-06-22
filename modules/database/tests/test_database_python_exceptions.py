"""Python integration tests for database awaitables (no live DB required)."""

import asyncio

import pytest
from oink_judge.pybind11_awaitable_support import AwaitableBridge
from oink_judge.pybind11_database import AsyncTableSubmissions


def test_load_submissions_auto_initializes_without_config():
    async def run() -> None:
        async with AwaitableBridge():
            with pytest.raises(RuntimeError, match="Could not open config file:"):
                await AsyncTableSubmissions.instance().async_load_submissions_by_user_and_problem(
                    "user", "problem"
                )

    asyncio.run(run())
