"""Python tests for database bindings (no live DB required)."""

import asyncio
import os

from oink_judge.pybind11_awaitable_support import AwaitableBridge
from oink_judge.pybind11_config import set_config_file_path, set_credentials_file_path
from oink_judge.pybind11_database import (
    ExecuteOptions,
    Statement,
    StatementsBlock,
    TableExecuteOptions,
    TableSubmissions,
)


def _configure_test_database() -> None:
    config_dir = os.environ["OINK_JUDGE_TEST_CONFIG_DIR"]
    set_config_file_path(f"{config_dir}/good_config.json")
    set_credentials_file_path(f"{config_dir}/good_credentials.json")


def test_load_submissions_returns_none_when_database_unavailable():
    _configure_test_database()

    async def run() -> None:
        async with AwaitableBridge():
            submissions = await TableSubmissions.instance().load_submissions_by_user_and_problem("user", "problem")
            assert submissions is None

    asyncio.run(run())


def test_execute_options_and_statements_block_api():
    options = ExecuteOptions(read_only=True, timeout_sec=5, retries=2)
    assert options.read_only is True
    assert options.timeout_sec == 5
    assert options.retries == 2

    table_options = TableExecuteOptions(read_only=True, timeout_sec=3, retries=1)
    assert table_options.execute_options.read_only is True
    assert table_options.execute_options.timeout_sec == 3
    assert table_options.execute_options.retries == 1

    block = StatementsBlock("users")
    block.add_statement(Statement("users__select", "SELECT 1"))
    assert block.name == "users"
    assert block.get_statement("users__select").sql == "SELECT 1"


def test_table_submissions_accepts_option_kwargs():
    _configure_test_database()

    async def run() -> None:
        async with AwaitableBridge():
            submissions = await TableSubmissions.instance().load_submissions_by_user_and_problem(
                "user",
                "problem",
                read_only=True,
                timeout_sec=1,
            )
            assert submissions is None

    asyncio.run(run())

