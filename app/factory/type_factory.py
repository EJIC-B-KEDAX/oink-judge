from __future__ import annotations

from typing import Any, Callable, Generic, TypeVar

T = TypeVar("T")


class TypeFactory(Generic[T]):
    _instances: dict[type, TypeFactory[Any]] = {}

    def __init__(self) -> None:
        self._registered: dict[str, Callable[..., T]] = {}

    @classmethod
    def instance(cls) -> TypeFactory[T]:
        if cls not in TypeFactory._instances:
            TypeFactory._instances[cls] = cls()
        return TypeFactory._instances[cls]  # type: ignore[return-value]

    def get_registered_types(self) -> dict[str, Callable[..., T]]:
        return self._registered

    def register_type(self, name: str, func: Callable[..., T]) -> None:
        self._registered[name] = func

    def create(self, name: str, *args: Any, **kwargs: Any) -> T:
        try:
            return self._registered[name](*args, **kwargs)
        except KeyError:
            raise KeyError(f"Unknown type: {name!r}") from None

    def register(self, name: str) -> Callable[[Callable[..., T]], Callable[..., T]]:
        def decorator(fn: Callable[..., T]) -> Callable[..., T]:
            self.register_type(name, fn)
            return fn

        return decorator
