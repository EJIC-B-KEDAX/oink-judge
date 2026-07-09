from typing import Annotated

from fastapi import Depends, HTTPException, Request

from app.auth.models import AuthUser
from app.auth.roles import ADMIN_ROLES, Role, can_login
from app.auth.sessions.redis_store import SessionStore
from app.auth.tables.users import AuthUsersTable


def _extract_session_id(request: Request) -> str | None:
    auth_header = request.headers.get("Authorization", "")
    if auth_header.startswith("Bearer "):
        token = auth_header.removeprefix("Bearer ").strip()
        if token:
            return token

    cookie = request.cookies.get("session_id")
    if cookie:
        return cookie

    return None


def _extract_refresh_token(
    request: Request, body_token: str | None = None
) -> str | None:
    if body_token:
        return body_token

    cookie = request.cookies.get("refresh_token")
    if cookie:
        return cookie

    return None


async def require_api_user(request: Request) -> AuthUser:
    session_id = _extract_session_id(request)
    if session_id is None:
        raise HTTPException(status_code=401, detail="Not authenticated")

    session_user = await SessionStore.instance().get(session_id)
    if session_user is None:
        raise HTTPException(status_code=401, detail="Invalid or expired session")

    db_user = await AuthUsersTable.instance().get_user(session_user.username)
    if db_user is None:
        raise HTTPException(status_code=401, detail="Invalid or expired session")
    if not can_login(db_user.role):
        raise HTTPException(
            status_code=403, detail="Account is not allowed to access the API"
        )

    return db_user


def require_admin(user: AuthUser) -> None:
    if user.role not in ADMIN_ROLES:
        raise HTTPException(status_code=403, detail="Admin access required")


def require_superadmin(user: AuthUser) -> None:
    if user.role != Role.SUPERADMIN:
        raise HTTPException(status_code=403, detail="Superadmin access required")


async def get_current_user(request: Request) -> AuthUser:
    return await require_api_user(request)


async def get_admin_user(
    user: Annotated[AuthUser, Depends(get_current_user)],
) -> AuthUser:
    require_admin(user)
    return user


async def get_superadmin_user(
    user: Annotated[AuthUser, Depends(get_current_user)],
) -> AuthUser:
    require_superadmin(user)
    return user


CurrentUser = Annotated[AuthUser, Depends(get_current_user)]
CurrentAdminUser = Annotated[AuthUser, Depends(get_admin_user)]
CurrentSuperadminUser = Annotated[AuthUser, Depends(get_superadmin_user)]
