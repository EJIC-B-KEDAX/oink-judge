from typing import Union

import grpc
import grpc.aio
from oink_judge.pybind11_content_service import (
    ContentChangeType,
    ManifestStorage,
    get_content_directory,
    get_permissions_from_manifest,
)
from oink_judge.pybind11_logger import log_info

from app.config import require_has_value
from app.services.content_service.content_manifest import compare_manifests
from app.services.content_service.content_service_stub import (
    ContentServiceStub,
    get_content_storage_stub,
)
from app.utils import (
    create_directory_if_not_exists,
    load_file,
    remove_file_or_directory,
    set_permissions,
    store_file,
)


def _to_bytes(content: Union[str, bytes]) -> bytes:
    if isinstance(content, bytes):
        return content
    return content.encode("latin-1")


def _to_str(content: Union[str, bytes]) -> str:
    if isinstance(content, str):
        return content
    return content.decode("latin-1")


class ContentStorage:
    _instance: "ContentStorage | None" = None

    def __init__(self, stub: ContentServiceStub | None = None) -> None:
        if stub is None:
            stub = require_has_value(get_content_storage_stub())
        self._stub = stub

    @classmethod
    def instance(cls) -> "ContentStorage":
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    async def sync_content(self, content_type: str, content_id: str) -> None:
        content_path = (
            require_has_value(get_content_directory(content_type)) / content_id
        )

        server_manifest = await self._get_manifest_from_server(content_type, content_id)

        local_manifest = ManifestStorage.instance().get_manifest(
            content_type, content_id
        )
        changes = compare_manifests(local_manifest, server_manifest)

        if not changes:
            return

        log_info(
            "content_storage",
            f"Syncing {len(changes)} change(s) for {content_type}/{content_id}",
        )

        for change in changes:
            relative_file_path = str(change.file_path)
            file_path = content_path / change.file_path
            if change.type in (ContentChangeType.ADDED, ContentChangeType.MODIFIED):
                file = await self._get_file_from_server(
                    content_type, content_id, relative_file_path
                )
                store_file(file_path, _to_str(file))
                permissions = get_permissions_from_manifest(
                    server_manifest, change.file_path
                )
                set_permissions(file_path, permissions)
            elif change.type == ContentChangeType.ATTRIBUTES_CHANGED:
                permissions = get_permissions_from_manifest(
                    server_manifest, change.file_path
                )
                set_permissions(file_path, permissions)
            elif change.type == ContentChangeType.REMOVED:
                remove_file_or_directory(file_path)

    async def sync_file(
        self, content_type: str, content_id: str, file_path: str
    ) -> None:
        from pathlib import Path

        content_path = (
            require_has_value(get_content_directory(content_type)) / content_id
        )
        target_file_path = Path(file_path)

        server_manifest = await self._get_manifest_from_server(content_type, content_id)

        local_manifest = ManifestStorage.instance().get_manifest(
            content_type, content_id
        )
        changes = [
            change
            for change in compare_manifests(local_manifest, server_manifest)
            if change.file_path == target_file_path
        ]

        if not changes:
            return

        log_info(
            "content_storage",
            f"Syncing file {target_file_path} for {content_type}/{content_id}",
        )

        for change in changes:
            relative_file_path = str(change.file_path)
            local_file_path = content_path / change.file_path
            if change.type in (ContentChangeType.ADDED, ContentChangeType.MODIFIED):
                file = await self._get_file_from_server(
                    content_type, content_id, relative_file_path
                )
                store_file(local_file_path, _to_str(file))
                permissions = get_permissions_from_manifest(
                    server_manifest, change.file_path
                )
                set_permissions(local_file_path, permissions)
            elif change.type == ContentChangeType.ATTRIBUTES_CHANGED:
                permissions = get_permissions_from_manifest(
                    server_manifest, change.file_path
                )
                set_permissions(local_file_path, permissions)
            elif change.type == ContentChangeType.REMOVED:
                remove_file_or_directory(local_file_path)

    async def update_content_on_server(
        self, content_type: str, content_id: str
    ) -> None:
        content_path = (
            require_has_value(get_content_directory(content_type)) / content_id
        )
        if not content_path.exists():
            raise RuntimeError(f"Content path does not exist: {content_path}")

        server_manifest = await self._get_manifest_from_server(content_type, content_id)

        local_manifest = ManifestStorage.instance().get_manifest(
            content_type, content_id
        )
        local_manifest_json = local_manifest.to_json()
        changes = compare_manifests(server_manifest, local_manifest_json)

        if not changes:
            return

        log_info(
            "content_storage",
            f"Uploading {len(changes)} change(s) for {content_type}/{content_id}",
        )

        for change in changes:
            relative_file_path = str(change.file_path)
            file_path = content_path / change.file_path
            if change.type == ContentChangeType.ADDED:
                file_content = _to_bytes(load_file(file_path))
                await self._create_file_on_server(
                    content_type, content_id, relative_file_path, file_content
                )
                await self._set_permissions_on_server(
                    content_type,
                    content_id,
                    relative_file_path,
                    get_permissions_from_manifest(
                        local_manifest_json, change.file_path
                    ),
                )
            elif change.type == ContentChangeType.MODIFIED:
                file_content = _to_bytes(load_file(file_path))
                await self._update_file_on_server(
                    content_type, content_id, relative_file_path, file_content
                )
                await self._set_permissions_on_server(
                    content_type,
                    content_id,
                    relative_file_path,
                    get_permissions_from_manifest(
                        local_manifest_json, change.file_path
                    ),
                )
            elif change.type == ContentChangeType.ATTRIBUTES_CHANGED:
                await self._set_permissions_on_server(
                    content_type,
                    content_id,
                    relative_file_path,
                    get_permissions_from_manifest(
                        local_manifest_json, change.file_path
                    ),
                )
            elif change.type == ContentChangeType.REMOVED:
                await self._remove_file_on_server(
                    content_type, content_id, relative_file_path
                )

    async def create_content(self, content_type: str, content_id: str) -> None:
        await self._create_content_on_server(content_type, content_id)
        create_directory_if_not_exists(
            require_has_value(get_content_directory(content_type)) / content_id
        )

    async def list_content(self, content_type: str) -> list[str]:
        return await self._list_content_on_server(content_type)

    async def _get_manifest_from_server(
        self, content_type: str, content_id: str
    ) -> dict:
        try:
            return await self._stub.get_manifest(content_type, content_id)
        except grpc.aio.AioRpcError as error:
            raise RuntimeError(
                f"Failed to get manifest from server: {error.details()}"
            ) from error

    async def _get_file_from_server(
        self, content_type: str, content_id: str, file_path: str
    ) -> bytes:
        try:
            return await self._stub.get_file(content_type, content_id, file_path)
        except grpc.aio.AioRpcError as error:
            raise RuntimeError(
                f"Failed to get file from server: {error.details()}"
            ) from error

    async def _create_file_on_server(
        self,
        content_type: str,
        content_id: str,
        file_path: str,
        file_content: bytes,
    ) -> None:
        try:
            await self._stub.create_file(
                content_type, content_id, file_path, file_content
            )
        except grpc.aio.AioRpcError as error:
            raise RuntimeError(
                f"Failed to create file on server: {error.details()}"
            ) from error

    async def _update_file_on_server(
        self,
        content_type: str,
        content_id: str,
        file_path: str,
        file_content: bytes,
    ) -> None:
        try:
            await self._stub.update_file(
                content_type, content_id, file_path, file_content
            )
        except grpc.aio.AioRpcError as error:
            raise RuntimeError(
                f"Failed to update file on server: {error.details()}"
            ) from error

    async def _remove_file_on_server(
        self, content_type: str, content_id: str, file_path: str
    ) -> None:
        try:
            await self._stub.delete_file(content_type, content_id, file_path)
        except grpc.aio.AioRpcError as error:
            raise RuntimeError(
                f"Failed to delete file on server: {error.details()}"
            ) from error

    async def _set_permissions_on_server(
        self,
        content_type: str,
        content_id: str,
        file_path: str,
        permissions: int,
    ) -> None:
        try:
            await self._stub.set_permissions(
                content_type, content_id, file_path, int(permissions)
            )
        except grpc.aio.AioRpcError as error:
            raise RuntimeError(
                f"Failed to set permissions on server: {error.details()}"
            ) from error

    async def _create_content_on_server(
        self, content_type: str, content_id: str
    ) -> None:
        try:
            await self._stub.create_content(content_type, content_id)
        except grpc.aio.AioRpcError as error:
            raise RuntimeError(
                f"Failed to create content on server: {error.details()}"
            ) from error

    async def _list_content_on_server(self, content_type: str) -> list[str]:
        try:
            return await self._stub.list_content(content_type)
        except grpc.aio.AioRpcError as error:
            raise RuntimeError(
                f"Failed to list content on server: {error.details()}"
            ) from error
