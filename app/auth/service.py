from dataclasses import dataclass
from datetime import UTC, datetime, timedelta

from app.auth.errors import AuthError
from app.auth.models import AuthUser
from app.auth.passwords import hash_password
from app.auth.roles import Role, can_login
from app.auth.sessions.redis_store import SessionStore
from app.auth.settings import get_refresh_token_ttl_seconds
from app.auth.tables.refresh_tokens import AuthRefreshTokensTable
from app.auth.tables.users import AuthUsersTable
from app.auth.tokens import generate_refresh_token, generate_session_id, hash_token
from app.auth.validation import validate_username


@dataclass(frozen=True, slots=True)
class AuthTokens:
    session_id: str
    refresh_token: str
    user: AuthUser


class AuthService:
    _instance: "AuthService | None" = None

    @classmethod
    def instance(cls) -> "AuthService":
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    async def register(self, username: str, password: str) -> AuthUser:
        username_error = validate_username(username)
        if username_error is not None:
            raise AuthError(422, username_error)

        users = AuthUsersTable.instance()
        if await users.user_exists(username):
            raise AuthError(409, "Username already taken")

        created = await users.register_user(
            username, hash_password(password), Role.USER
        )
        if not created:
            raise AuthError(409, "Username already taken")

        user = await users.get_user(username)
        if user is None:
            raise AuthError(500, "Failed to create user")

        return user

    async def login(self, username: str, password: str) -> AuthTokens:
        user = await AuthUsersTable.instance().authenticate(username, password)
        if user is None:
            raise AuthError(401, "Invalid credentials")
        if not can_login(user.role):
            raise AuthError(403, "Account is not allowed to log in")

        return await self._issue_tokens(user)

    async def refresh(self, refresh_token: str) -> AuthTokens:
        token_hash = hash_token(refresh_token)
        username = await AuthRefreshTokensTable.instance().validate(token_hash)
        if username is None:
            raise AuthError(401, "Invalid or expired refresh token")

        user = await AuthUsersTable.instance().get_user(username)
        if user is None or not can_login(user.role):
            raise AuthError(401, "Invalid or expired refresh token")

        await AuthRefreshTokensTable.instance().revoke(token_hash)
        return await self._issue_tokens(user)

    async def logout(self, session_id: str | None, refresh_token: str | None) -> None:
        if session_id is not None:
            await SessionStore.instance().delete(session_id)
        if refresh_token is not None:
            await AuthRefreshTokensTable.instance().revoke(hash_token(refresh_token))

    async def delete_account(self, user: AuthUser) -> None:
        if not await AuthUsersTable.instance().soft_delete(user.username):
            raise AuthError(500, "Failed to delete account")

        await AuthRefreshTokensTable.instance().revoke_all_for_user(user.username)
        await SessionStore.instance().delete_all_for_user(user.username)

    async def _issue_tokens(self, user: AuthUser) -> AuthTokens:
        session_id = generate_session_id()
        refresh_token = generate_refresh_token()
        expires_at = datetime.now(tz=UTC) + timedelta(
            seconds=get_refresh_token_ttl_seconds()
        )

        await SessionStore.instance().create(session_id, user)
        await AuthRefreshTokensTable.instance().create(
            hash_token(refresh_token),
            user.username,
            expires_at,
        )

        return AuthTokens(
            session_id=session_id,
            refresh_token=refresh_token,
            user=user,
        )
