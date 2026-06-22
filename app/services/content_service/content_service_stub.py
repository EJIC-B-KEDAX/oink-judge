from typing import AsyncIterator, Optional

import grpc
import grpc.aio
from oink_judge import content_service_pb2 as pb2
from oink_judge import content_service_pb2_grpc as pb2_grpc
from oink_judge.pybind11_content_service import get_content_storage_stub_type

import app.utils.grpc  # noqa: F401 — registers channel/credential factory types
from app.factory import ParameterizedTypeFactory, normalize_argument, parse_parameters
from app.utils.grpc import ChannelFactory

CHUNK_SIZE = 64 * 1024  # 64 KB


class ContentServiceStubFactory(ParameterizedTypeFactory["ContentServiceStub"]): ...


def get_content_storage_stub() -> Optional["ContentServiceStub"]:
    stub_type = get_content_storage_stub_type()
    if stub_type is None:
        return None
    return ContentServiceStubFactory.instance().create(stub_type)


def register_content_service_channel_stub_type() -> None:
    def channel_stub(params: str) -> "ContentServiceStub":
        parts = parse_parameters(params, ",")
        if len(parts) != 1:
            raise ValueError(
                "Expected exactly one parameter for content_service_stub: channel type"
            )
        channel_spec = normalize_argument(parts[0], True)
        channel = ChannelFactory.instance().create(channel_spec)
        return ContentServiceStub(channel)

    ContentServiceStubFactory.instance().register_type(
        "content_service_stub", channel_stub
    )


register_content_service_channel_stub_type()


class ContentServiceStub:
    _stub: pb2_grpc.ContentServiceStub

    def __init__(self, channel: grpc.aio.Channel) -> None:
        self._stub = pb2_grpc.ContentServiceStub(channel)

    async def get_manifest(self, content_type: str, content_id: str) -> dict:
        import json

        request = pb2.GetManifestRequest(
            content_type=content_type,
            content_id=content_id,
        )
        parts: list[str] = []
        async for response in self._stub.GetManifest(request):
            parts.append(response.manifest)

        return json.loads("".join(parts))

    async def get_file(
        self, content_type: str, content_id: str, file_path: str
    ) -> bytes:
        request = pb2.GetFileRequest(
            content_type=content_type,
            content_id=content_id,
            file_path=file_path,
        )
        chunks: list[bytes] = []
        async for response in self._stub.GetFile(request):
            chunks.append(response.file_chunk.data)

        return b"".join(chunks)

    async def create_file(
        self,
        content_type: str,
        content_id: str,
        file_path: str,
        file_content: bytes,
    ) -> None:
        await self._stub.CreateFile(
            self._file_upload_requests(
                pb2.CreateFileRequest,
                content_type,
                content_id,
                file_path,
                file_content,
            )
        )

    async def update_file(
        self,
        content_type: str,
        content_id: str,
        file_path: str,
        file_content: bytes,
    ) -> None:
        await self._stub.UpdateFile(
            self._file_upload_requests(
                pb2.UpdateFileRequest,
                content_type,
                content_id,
                file_path,
                file_content,
            )
        )

    async def delete_file(
        self, content_type: str, content_id: str, file_path: str
    ) -> None:
        request = pb2.DeleteFileRequest(
            content_type=content_type,
            content_id=content_id,
            file_path=file_path,
        )
        await self._stub.DeleteFile(request)

    @staticmethod
    async def _file_upload_requests(
        request_cls,  # CreateFileRequest | UpdateFileRequest
        content_type: str,
        content_id: str,
        file_path: str,
        file_content: bytes,
    ) -> AsyncIterator:
        # First message: metadata
        info = pb2.FileInfo(
            content_type=content_type,
            content_id=content_id,
            file_path=file_path,
        )
        yield request_cls(file_info=info)

        # Subsequent messages: data chunks
        for offset in range(0, len(file_content), CHUNK_SIZE):
            chunk = pb2.FileChunk(data=file_content[offset : offset + CHUNK_SIZE])
            yield request_cls(file_chunk=chunk)
