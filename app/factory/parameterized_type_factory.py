from __future__ import annotations

from typing import Any, TypeVar

from oink_judge.pybind11_factory import parse_name

from .type_factory import TypeFactory

T = TypeVar("T")


class ParameterizedTypeFactory(TypeFactory[T]):
    @classmethod
    def instance(cls) -> ParameterizedTypeFactory[T]:
        return super().instance()  # type: ignore[return-value]

    def create(self, name: str, *args: Any, **kwargs: Any) -> T:
        name, params = parse_name(name)
        return super().create(name, params, *args, **kwargs)
