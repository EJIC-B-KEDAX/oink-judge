"""Integration tests: Python content_service client against the C++ gRPC server."""

from __future__ import annotations

import asyncio
import json
import os
import socket
import subprocess
import time
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path

import grpc
import pytest
from oink_judge.pybind11_config import set_config_file_path, set_credentials_file_path

RESOURCES = Path(__file__).parent / "resources" / "test_content_service_server"
SERVER_EXE_ENV = "CONTENT_SERVICE_TEST_SERVER"


def _free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def _wait_for_server(listen_address: str, timeout_seconds: float = 10.0) -> None:
    deadline = time.monotonic() + timeout_seconds
    last_error: Exception | None = None
    while time.monotonic() < deadline:
        try:
            grpc.channel_ready_future(grpc.insecure_channel(listen_address)).result(
                timeout=1.0
            )
            return
        except grpc.FutureTimeoutError as error:
            last_error = error
    raise RuntimeError(
        f"Content service test server at {listen_address} did not become ready"
    ) from last_error


@dataclass
class IntegrationEnv:
    listen_address: str
    client_problems_dir: Path
    server_problems_dir: Path
    process: subprocess.Popen[str]


@pytest.fixture(scope="module")
def integration_env(
    tmp_path_factory: pytest.TempPathFactory,
) -> Iterator[IntegrationEnv]:
    server_exe = os.environ.get(SERVER_EXE_ENV)
    if server_exe is None or not Path(server_exe).is_file():
        pytest.skip(f"{SERVER_EXE_ENV} is not set or executable was not built")

    root = tmp_path_factory.mktemp("content_service_python_integration")
    client_problems_dir = root / "client_problems"
    client_problems_dir.mkdir()

    server_problems_dir = RESOURCES / "problems"
    server_mut_dir = server_problems_dir / "mut"
    server_mut_dir.mkdir(exist_ok=True)
    (server_mut_dir / "existing.txt").write_text("seed content")

    port = _free_port()
    listen_address = f"127.0.0.1:{port}"

    server_config_path = root / "server_config.json"
    server_config_path.write_text(
        json.dumps(
            {
                "directories": {"problems": str(server_problems_dir.resolve())},
                "timings": {"full_rescan_interval": 0.0},
            }
        )
    )

    credentials_path = root / "credentials.json"
    credentials_path.write_text("{}")

    client_config_path = root / "client_config.json"
    client_config_path.write_text(
        json.dumps(
            {
                "directories": {"problems": str(client_problems_dir.resolve())},
                "timings": {"full_rescan_interval": 0.0},
                "content_storage": {
                    "stub_type": f"content_service_stub(insecure({listen_address}))"
                },
            }
        )
    )

    set_config_file_path(str(client_config_path))
    set_credentials_file_path(str(credentials_path))

    process = subprocess.Popen(  # noqa: S603
        [
            server_exe,
            str(server_config_path),
            str(credentials_path),
            listen_address,
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    try:
        _wait_for_server(listen_address)
    except Exception:
        process.terminate()
        stdout, stderr = process.communicate(timeout=5)
        pytest.fail(
            "Failed to start content service test server\n"
            f"stdout:\n{stdout}\nstderr:\n{stderr}"
        )

    yield IntegrationEnv(
        listen_address=listen_address,
        client_problems_dir=client_problems_dir,
        server_problems_dir=server_problems_dir,
        process=process,
    )

    process.terminate()
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)

    if (server_mut_dir / "uploaded.txt").exists():
        (server_mut_dir / "uploaded.txt").unlink()
    if (server_mut_dir / "existing.txt").exists():
        (server_mut_dir / "existing.txt").unlink()


class TestPythonStubAgainstCppServer:
    def test_get_manifest(self, integration_env: IntegrationEnv) -> None:
        from app.services.content_service import get_content_storage_stub

        async def run() -> dict:
            stub = get_content_storage_stub()
            assert stub is not None
            return await stub.get_manifest("problem", "1")

        manifest = asyncio.run(run())
        assert "files" in manifest
        assert "input.txt" in manifest["files"]
        assert "subdir/nested.txt" in manifest["files"]

    def test_get_file(self, integration_env: IntegrationEnv) -> None:
        from app.services.content_service import get_content_storage_stub

        async def run() -> bytes:
            stub = get_content_storage_stub()
            assert stub is not None
            return await stub.get_file("problem", "1", "input.txt")

        content = asyncio.run(run())
        expected = (RESOURCES / "problems" / "1" / "input.txt").read_bytes()
        assert content == expected


class TestPythonContentStorageAgainstCppServer:
    def test_ensure_content_exists_downloads_from_server(
        self, integration_env: IntegrationEnv
    ) -> None:
        from app.services.content_service import (
            ContentStorage,
            get_content_storage_stub,
        )

        async def run() -> None:
            storage = ContentStorage(stub=get_content_storage_stub())
            await storage.ensure_content_exists("problem", "1")

        asyncio.run(run())

        local_problem_dir = integration_env.client_problems_dir / "1"
        expected_input = (RESOURCES / "problems" / "1" / "input.txt").read_text()
        expected_nested = (
            RESOURCES / "problems" / "1" / "subdir" / "nested.txt"
        ).read_text()
        assert (local_problem_dir / "input.txt").read_text() == expected_input
        assert (
            local_problem_dir / "subdir" / "nested.txt"
        ).read_text() == expected_nested
        assert (local_problem_dir / "manifest.json").exists()

    def test_update_content_on_server_uploads_new_file(
        self, integration_env: IntegrationEnv
    ) -> None:
        from app.services.content_service import (
            ContentStorage,
            get_content_storage_stub,
        )

        local_mut_dir = integration_env.client_problems_dir / "mut"
        local_mut_dir.mkdir()
        uploaded_path = local_mut_dir / "uploaded.txt"
        uploaded_path.write_bytes(b"uploaded from python client")

        async def run() -> None:
            storage = ContentStorage(stub=get_content_storage_stub())
            await storage.update_content_on_server("problem", "mut")

        asyncio.run(run())

        server_file = integration_env.server_problems_dir / "mut" / "uploaded.txt"
        assert server_file.read_bytes() == b"uploaded from python client"

        async def verify_via_stub() -> bytes:
            stub = get_content_storage_stub()
            assert stub is not None
            return await stub.get_file("problem", "mut", "uploaded.txt")

        assert asyncio.run(verify_via_stub()) == b"uploaded from python client"
