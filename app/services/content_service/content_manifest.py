import hashlib
import json
import os
import time
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Any, Dict, List, Optional


class ContentChangeType(Enum):
    ADDED = "ADDED"
    MODIFIED = "MODIFIED"
    REMOVED = "REMOVED"
    ATTRIBUTES_CHANGED = "ATTRIBUTES_CHANGED"


@dataclass
class ContentChange:
    type: ContentChangeType
    file_path: str


class ContentManifest:
    def __init__(
        self, content_type: str, content_id: str, full_rescan_interval: int = 3600
    ):
        """
        Initialize ContentManifest.

        Args:
            content_type: Type of content
            content_id: ID of content
            full_rescan_interval: Interval in seconds for full rescan (default: 3600)
        """
        self.content_type_ = content_type
        self.content_id_ = content_id
        self.last_updated_ = time.time()
        self.last_full_rescan_ = time.time()
        self.full_rescan_interval_ = full_rescan_interval

    def get_content_type(self) -> str:
        """Get content type."""
        return self.content_type_

    def get_content_id(self) -> str:
        """Get content ID."""
        return self.content_id_

    def to_string(self) -> str:
        """Get manifest as JSON string."""
        self.update_manifest()
        manifest_path = self.get_path_to_manifest_file()
        if manifest_path.exists():
            return manifest_path.read_text()
        return "{}"

    def to_json(self) -> Dict[str, Any]:
        """Get manifest as JSON object."""
        self.update_manifest()
        return self.stored_manifest_to_json()

    def get_path_to_manifest_file(self) -> Path:
        """Get path to manifest.json file."""
        content_dir = self._get_path_to_content_directory()
        return content_dir / "manifest.json"

    def update_manifest(self) -> None:
        """Update manifest by performing full or fast rescan."""
        started_update_on = time.time()

        if time.time() - self.last_full_rescan_ >= self.full_rescan_interval_:
            self.last_full_rescan_ = time.time()
            self.full_rescan_content()
        else:
            self.fast_rescan_content()

        self.last_updated_ = started_update_on

    def full_rescan_content(self) -> None:
        """Perform full rescan of content directory."""
        content_directory = self._get_path_to_content_directory()
        content_directory.mkdir(parents=True, exist_ok=True)

        manifest_json = {"files": {}}

        for entry in content_directory.rglob("*"):
            if entry.is_file():
                relative_path = str(entry.relative_to(content_directory))

                if relative_path == "manifest.json":
                    continue

                stat_info = entry.stat()

                cur_file_json = {
                    "size": stat_info.st_size,
                    "last_modified": int(stat_info.st_mtime),
                    "permissions": stat_info.st_mode & 0o777,
                    "sha256": self._calculate_sha256(entry),
                }

                manifest_json["files"][relative_path] = cur_file_json

        manifest_path = self.get_path_to_manifest_file()
        manifest_path.write_text(json.dumps(manifest_json, indent=4))

    def fast_rescan_content(self) -> None:
        """Perform fast rescan of content directory (reuses SHA256 for unchanged files)."""
        content_directory = self._get_path_to_content_directory()
        content_directory.mkdir(parents=True, exist_ok=True)

        new_manifest_json = {"files": {}}
        stored_manifest = self.stored_manifest_to_json()

        for entry in content_directory.rglob("*"):
            if entry.is_file():
                relative_path = str(entry.relative_to(content_directory))

                if relative_path == "manifest.json":
                    continue

                stat_info = entry.stat()

                cur_file_json: Dict[str, Any] = {
                    "size": stat_info.st_size,
                    "last_modified": int(stat_info.st_mtime),
                    "permissions": stat_info.st_mode & 0o777,
                }

                # Check if file unchanged - reuse SHA256
                if (
                    stored_manifest.get("files", {}).get(relative_path)
                    and stored_manifest["files"][relative_path]["size"]
                    == cur_file_json["size"]
                    and stored_manifest["files"][relative_path]["last_modified"]
                    == cur_file_json["last_modified"]
                ):
                    cur_file_json["sha256"] = stored_manifest["files"][relative_path][
                        "sha256"
                    ]
                else:
                    cur_file_json["sha256"] = self._calculate_sha256(entry)

                new_manifest_json["files"][relative_path] = cur_file_json

        manifest_path = self.get_path_to_manifest_file()
        manifest_path.write_text(json.dumps(new_manifest_json, indent=4))

    def stored_manifest_to_json(self) -> Dict[str, Any]:
        """Load stored manifest from file."""
        try:
            manifest_path = self.get_path_to_manifest_file()
            if manifest_path.exists():
                return json.loads(manifest_path.read_text())
        except Exception:
            pass
        return {}

    def _get_path_to_content_directory(self) -> Path:
        """Get path to content directory."""
        # TODO: Replace with actual content directory resolution from config
        base_path = Path(os.environ.get("CONTENT_BASE_PATH", "/tmp/oink-content"))
        return base_path / self.content_type_ / self.content_id_

    @staticmethod
    def _calculate_sha256(file_path: Path) -> str:
        """Calculate SHA256 hash of file."""
        sha256_hash = hashlib.sha256()
        with open(file_path, "rb") as f:
            for byte_block in iter(lambda: f.read(4096), b""):
                sha256_hash.update(byte_block)
        return sha256_hash.hexdigest()


def get_manifest_signature(content_type: str, content_id: str) -> str:
    """Get manifest signature (path to content directory)."""
    # TODO: Replace with actual content directory resolution from config
    base_path = Path(os.environ.get("CONTENT_BASE_PATH", "/tmp/oink-content"))
    return str(base_path / content_type / content_id)


def compare_manifests(
    old_manifest, new_manifest: Dict[str, Any]
) -> List[ContentChange]:
    """
    Compare two manifests and return list of changes.

    Args:
        old_manifest: Old manifest (ContentManifest object or dict)
        new_manifest: New manifest as dict

    Returns:
        List of ContentChange objects
    """
    if isinstance(old_manifest, ContentManifest):
        old_manifest = old_manifest.to_json()

    changes = []

    # Check for added or modified files
    if "files" in new_manifest:
        for file_path, new_file_info in new_manifest["files"].items():
            if "files" not in old_manifest or file_path not in old_manifest["files"]:
                changes.append(ContentChange(ContentChangeType.ADDED, file_path))
            else:
                old_file_info = old_manifest["files"][file_path]
                if old_file_info["sha256"] != new_file_info["sha256"]:
                    changes.append(ContentChange(ContentChangeType.MODIFIED, file_path))
                elif old_file_info["permissions"] != new_file_info["permissions"]:
                    changes.append(
                        ContentChange(ContentChangeType.ATTRIBUTES_CHANGED, file_path)
                    )

    # Check for removed files
    if "files" in old_manifest:
        for file_path in old_manifest["files"].keys():
            if "files" not in new_manifest or file_path not in new_manifest["files"]:
                changes.append(ContentChange(ContentChangeType.REMOVED, file_path))

    return changes


def get_permissions_from_manifest(
    manifest: Dict[str, Any], file_path: Path
) -> Optional[int]:
    """Get file permissions from manifest."""
    file_path_str = str(file_path)
    if (
        "files" in manifest
        and file_path_str in manifest["files"]
        and "permissions" in manifest["files"][file_path_str]
    ):
        return manifest["files"][file_path_str]["permissions"]
    return None
