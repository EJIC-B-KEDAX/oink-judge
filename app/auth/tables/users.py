from oink_judge.pybind11_database import (
    execute,
    execute_sql,
    get_default_executor,
)

from app.auth.models import AuthUser
from app.auth.passwords import verify_password
from app.auth.roles import Role


class AuthUsersTable:
    _instance: "AuthUsersTable | None" = None

    def __init__(self) -> None:
        self._initialized = False

    @classmethod
    def instance(cls) -> "AuthUsersTable":
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    async def _require_initialized(self) -> None:
        if not self._initialized:
            await self.initialize()

    async def initialize(self) -> None:
        if self._initialized:
            return

        await execute_sql(
            "CREATE TABLE IF NOT EXISTS auth_users ("
            "username TEXT PRIMARY KEY, "
            "password_hash TEXT NOT NULL, "
            "role TEXT NOT NULL, "
            "created_at TIMESTAMPTZ NOT NULL DEFAULT NOW());"
        )

        executor = get_default_executor()
        executor.prepare_statement(
            "auth_users__select_by_username",
            "SELECT username, password_hash, role FROM auth_users WHERE username = $1",
        )
        executor.prepare_statement(
            "auth_users__insert_user",
            "INSERT INTO auth_users (username, password_hash, role) VALUES ($1, $2, $3)",
        )
        executor.prepare_statement(
            "auth_users__update_role",
            "UPDATE auth_users SET role = $2 WHERE username = $1",
        )
        self._initialized = True

    async def user_exists(self, username: str) -> bool:
        await self._require_initialized()
        result = await execute(
            "auth_users__select_by_username", username, read_only=True
        )
        return not result.empty()

    async def register_user(
        self, username: str, password_hash: str, role: Role
    ) -> bool:
        await self._require_initialized()
        if await self.user_exists(username):
            return False

        await execute("auth_users__insert_user", username, password_hash, role.value)
        return True

    async def get_user(self, username: str) -> AuthUser | None:
        await self._require_initialized()
        result = await execute(
            "auth_users__select_by_username", username, read_only=True
        )
        if result.empty():
            return None

        row = result[0]
        return AuthUser(
            username=row["username"].as_string(),
            role=Role(row["role"].as_string()),
        )

    async def authenticate(self, username: str, password: str) -> AuthUser | None:
        await self._require_initialized()
        result = await execute(
            "auth_users__select_by_username", username, read_only=True
        )
        if result.empty():
            return None

        row = result[0]
        stored_hash = row["password_hash"].as_string()
        if not verify_password(password, stored_hash):
            return None

        return AuthUser(
            username=row["username"].as_string(),
            role=Role(row["role"].as_string()),
        )

    async def set_role(self, username: str, role: Role) -> bool:
        await self._require_initialized()
        if not await self.user_exists(username):
            return False

        await execute("auth_users__update_role", username, role.value)
        return True

    async def soft_delete(self, username: str) -> bool:
        return await self.set_role(username, Role.DELETED)
