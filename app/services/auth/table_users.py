import hashlib

from oink_judge.pybind11_database import (
    ConnectionPool,
    async_execute,
    async_execute_read_only,
    async_execute_sql,
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

    async def initialize(self) -> None:
        if self._initialized:
            return

        await async_execute_sql(
            "CREATE TABLE IF NOT EXISTS users ("
            "username TEXT PRIMARY KEY,"
            "password TEXT);",
            [],
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
        password_hash = hashlib.sha256(password.encode("latin1")).hexdigest()
        result = await async_execute_read_only("users__select_password", [username])

        if result.empty():
            return False
        if result.size() > 1:
            raise RuntimeError("User must have exactly one password")

        stored_password_hash = result[0]["password"].as_string()
        return stored_password_hash == password_hash

    async def register_user(self, username: str, password: str) -> bool:
        if await self.user_exists(username):
            return False

        password_hash = hashlib.sha256(password.encode()).hexdigest()
        await async_execute("users__insert_user", [username, password_hash])
        return True

    async def user_exists(self, username: str) -> bool:
        result = await async_execute_read_only("users__select_password", [username])
        return not result.empty()

    async def delete_user(self, username: str) -> bool:
        if not await self.user_exists(username):
            return False

        await async_execute("users__delete_user", [username])
        return True

    async def update_password(self, username: str, new_password: str) -> bool:
        if not await self.user_exists(username):
            return False

        password_hash = hashlib.sha256(new_password.encode()).hexdigest()
        await async_execute("users__update_user_password", [username, password_hash])
        return True
