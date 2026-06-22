from typing import Any

from oink_judge.pybind11_content_service import (
    ContentChange,
    ContentManifest,
    _compare_manifests_json_json,
    _compare_manifests_manifest_json,
)


def compare_manifests(
    old_manifest: ContentManifest | dict[str, Any],
    new_manifest: dict[str, Any],
) -> list[ContentChange]:
    if isinstance(old_manifest, ContentManifest):
        return _compare_manifests_manifest_json(old_manifest, new_manifest)
    return _compare_manifests_json_json(old_manifest, new_manifest)
