from dataclasses import dataclass

from app.auth.roles import Role


@dataclass(frozen=True, slots=True)
class AuthUser:
    username: str
    role: Role
