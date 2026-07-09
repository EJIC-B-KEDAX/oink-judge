import json
from typing import Any, cast

import redis.asyncio as redis

from app.auth.models import AuthUser
from app.auth.roles import Role
from app.auth.settings import (
    get_redis_host,
    get_redis_password,
    get_redis_port,
    get_session_ttl_seconds,
)


class SessionStore:
    _instance: "SessionStore | None" = None

    def __init__(self) -> None:
        self._client: redis.Redis | None = None

    @classmethod
    def instance(cls) -> "SessionStore":
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    def _get_client(self) -> redis.Redis:
        if self._client is None:
            self._client = redis.Redis(
                host=get_redis_host(),
                port=get_redis_port(),
                password=get_redis_password(),
                decode_responses=True,
            )
        return self._client

    @staticmethod
    def _session_key(session_id: str) -> str:
        return f"session:{session_id}"

    @staticmethod
    def _user_sessions_key(username: str) -> str:
        return f"user_sessions:{username}"

    async def create(self, session_id: str, user: AuthUser) -> None:
        client = self._get_client()
        payload = json.dumps({"username": user.username, "role": user.role.value})
        ttl = get_session_ttl_seconds()
        await client.set(self._session_key(session_id), payload, ex=ttl)
        await client.sadd(self._user_sessions_key(user.username), session_id)
        await client.expire(self._user_sessions_key(user.username), ttl)

    async def get(self, session_id: str) -> AuthUser | None:
        client = self._get_client()
        payload = await client.get(self._session_key(session_id))
        if payload is None:
            return None

        data: dict[str, Any] = json.loads(payload)
        return AuthUser(username=data["username"], role=Role(data["role"]))

    async def delete(self, session_id: str) -> None:
        client = self._get_client()
        payload = await client.get(self._session_key(session_id))
        if payload is not None:
            data = json.loads(payload)
            await client.srem(self._user_sessions_key(data["username"]), session_id)
        await client.delete(self._session_key(session_id))

    async def delete_all_for_user(self, username: str) -> None:
        client = self._get_client()
        session_ids = cast(
            set[str], await client.smembers(self._user_sessions_key(username))
        )
        if session_ids:
            await client.delete(
                *(self._session_key(session_id) for session_id in session_ids)
            )
        await client.delete(self._user_sessions_key(username))
