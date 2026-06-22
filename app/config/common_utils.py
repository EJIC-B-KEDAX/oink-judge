from typing import TypeVar

T = TypeVar("T")


def require_has_value(value: T | None, message: str = "optional expected with value") -> T:
    if value is None:
        raise RuntimeError(message)
    return value
