import time

from oink_judge.pybind11_database import (
    ConnectionPool,
    execute,
    execute_sql,
)

from app.services.auth.session import Session


class TableSessions:
    _instance: "TableSessions | None" = None

    def __init__(self) -> None:
        self._initialized = False

    @classmethod
    def instance(cls) -> "TableSessions":
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
            "CREATE TABLE IF NOT EXISTS sessions ("
            "id TEXT PRIMARY KEY, "
            "username TEXT, "
            "expire_at INTEGER);"
        )

        pool = ConnectionPool.instance()
        pool.prepare_statement(
            "sessions__add_session",
            "INSERT INTO sessions (id, username, expire_at) VALUES ($1, $2, $3)",
        )
        pool.prepare_statement(
            "sessions__remove_session",
            "DELETE FROM sessions WHERE id = $1",
        )
        pool.prepare_statement(
            "sessions__remove_by_username",
            "DELETE FROM sessions WHERE username = $1",
        )
        pool.prepare_statement(
            "sessions__select_session",
            "SELECT username, expire_at FROM sessions WHERE id = $1",
        )
        self._initialized = True

    async def add_session(self, session: Session) -> bool:
        await self._require_initialized()
        await execute(
            "sessions__add_session",
            session.session_id,
            session.username,
            session.expire_at,
        )
        return True

    async def remove_session(self, session_id: str) -> bool:
        await self._require_initialized()
        await execute("sessions__remove_session", session_id)
        return True

    async def whose_session(self, session_id: str) -> str:
        await self._require_initialized()
        result = await execute("sessions__select_session", session_id)

        if result.empty():
            return ""
        if result.size() > 1:
            raise RuntimeError("Session ID must contain only one session")

        row = result[0]
        stored_username = row["username"].as_string()
        stored_expire_at = row["expire_at"].as_int64()

        if self._is_expired(stored_expire_at):
            await execute("sessions__remove_session", session_id)
            return ""

        return stored_username

    @staticmethod
    def _is_expired(expire_at: int) -> bool:
        return time.time() > expire_at
