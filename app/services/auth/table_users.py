import hashlib

from oink_judge.pybind11_database import (
    ConnectionPool,
    execute,
    execute_sql,
)


class TableUsers:
    _instance: "TableUsers | None" = None

    def __init__(self) -> None:
        self._initialized = False

    @classmethod
    def instance(cls) -> "TableUsers":
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
            "CREATE TABLE IF NOT EXISTS users ("
            "username TEXT PRIMARY KEY,"
            "password TEXT);"
        )

        pool = ConnectionPool.instance()
        pool.prepare_statement(
            "users__select_password",
            "SELECT password FROM users WHERE username = $1",
        )
        pool.prepare_statement(
            "users__insert_user",
            "INSERT INTO users (username, password) VALUES ($1, $2)",
        )
        pool.prepare_statement(
            "users__delete_user",
            "DELETE FROM users WHERE username = $1",
        )
        pool.prepare_statement(
            "users__update_user_password",
            "UPDATE users SET password = $2 WHERE username = $1",
        )
        self._initialized = True

    async def authenticate(self, username: str, password: str) -> bool:
        await self._require_initialized()
        password_hash = hashlib.sha256(password.encode()).hexdigest()
        result = await execute("users__select_password", username, read_only=True)

        if result.empty():
            return False
        if result.size() > 1:
            raise RuntimeError("User must have exactly one password")

        stored_password_hash = result[0]["password"].as_string()
        return stored_password_hash == password_hash

    async def register_user(self, username: str, password: str) -> bool:
        await self._require_initialized()
        if await self.user_exists(username):
            return False

        password_hash = hashlib.sha256(password.encode()).hexdigest()
        await execute("users__insert_user", username, password_hash)
        return True

    async def user_exists(self, username: str) -> bool:
        await self._require_initialized()
        result = await execute("users__select_password", username, read_only=True)
        return not result.empty()

    async def delete_user(self, username: str) -> bool:
        await self._require_initialized()
        if not await self.user_exists(username):
            return False

        await execute("users__delete_user", username)
        return True

    async def update_password(self, username: str, new_password: str) -> bool:
        await self._require_initialized()
        if not await self.user_exists(username):
            return False

        password_hash = hashlib.sha256(new_password.encode()).hexdigest()
        await execute("users__update_user_password", username, password_hash)
        return True
