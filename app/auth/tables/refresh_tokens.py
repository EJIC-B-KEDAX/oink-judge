from datetime import UTC, datetime

from oink_judge.pybind11_database import (
    ConnectionPool,
    execute,
    execute_sql,
)


class AuthRefreshTokensTable:
    _instance: "AuthRefreshTokensTable | None" = None

    def __init__(self) -> None:
        self._initialized = False

    @classmethod
    def instance(cls) -> "AuthRefreshTokensTable":
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
            "CREATE TABLE IF NOT EXISTS auth_refresh_tokens ("
            "token_hash TEXT PRIMARY KEY, "
            "username TEXT NOT NULL, "
            "expires_at TIMESTAMPTZ NOT NULL, "
            "revoked_at TIMESTAMPTZ);"
        )

        pool = ConnectionPool.instance()
        pool.prepare_statement(
            "auth_refresh_tokens__insert",
            "INSERT INTO auth_refresh_tokens (token_hash, username, expires_at) VALUES ($1, $2, $3)",
        )
        pool.prepare_statement(
            "auth_refresh_tokens__select",
            "SELECT username, expires_at, revoked_at FROM auth_refresh_tokens WHERE token_hash = $1",
        )
        pool.prepare_statement(
            "auth_refresh_tokens__revoke",
            "UPDATE auth_refresh_tokens SET revoked_at = NOW() WHERE token_hash = $1",
        )
        pool.prepare_statement(
            "auth_refresh_tokens__revoke_all_for_user",
            "UPDATE auth_refresh_tokens SET revoked_at = NOW() "
            "WHERE username = $1 AND revoked_at IS NULL",
        )
        self._initialized = True

    async def create(self, token_hash: str, username: str, expires_at: datetime) -> None:
        await self._require_initialized()
        await execute(
            "auth_refresh_tokens__insert",
            token_hash,
            username,
            expires_at.replace(tzinfo=UTC).isoformat(),
        )

    async def validate(self, token_hash: str) -> str | None:
        await self._require_initialized()
        result = await execute("auth_refresh_tokens__select", token_hash, read_only=True)
        if result.empty():
            return None

        row = result[0]
        if not row["revoked_at"].is_null():
            return None

        expires_at_raw = row["expires_at"].as_string()
        expires_at = datetime.fromisoformat(expires_at_raw.replace("Z", "+00:00"))
        if expires_at.tzinfo is None:
            expires_at = expires_at.replace(tzinfo=UTC)
        if datetime.now(tz=UTC) >= expires_at:
            return None

        return row["username"].as_string()

    async def revoke(self, token_hash: str) -> None:
        await self._require_initialized()
        await execute("auth_refresh_tokens__revoke", token_hash)

    async def revoke_all_for_user(self, username: str) -> None:
        await self._require_initialized()
        await execute("auth_refresh_tokens__revoke_all_for_user", username)
