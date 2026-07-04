import asyncio
import os

import pytest

pytestmark = pytest.mark.skipif(
    os.getenv("OINK_JUDGE_RUN_DATABASE_TESTS") != "1",
    reason="Set OINK_JUDGE_RUN_DATABASE_TESTS=1 to run database python integration tests",
)


def _configure_test_database() -> None:
    config_dir = os.environ["OINK_JUDGE_TEST_CONFIG_DIR"]
    from oink_judge.pybind11_config import set_config_file_path, set_credentials_file_path

    set_config_file_path(f"{config_dir}/good_config.json")
    set_credentials_file_path(f"{config_dir}/good_credentials.json")


def test_connection_pool_initialize():
    from oink_judge.pybind11_awaitable_support import AwaitableBridge
    from oink_judge.pybind11_database import ConnectionPool

    _configure_test_database()

    async def run() -> None:
        async with AwaitableBridge():
            await asyncio.wait_for(ConnectionPool.instance().initialize(), timeout=15.0)

    asyncio.run(run())
