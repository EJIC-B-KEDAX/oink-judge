from fastapi import APIRouter, HTTPException, Request, Response

from app.auth.api.deps import (
    CurrentUser,
    _extract_refresh_token,
    _extract_session_id,
)
from app.auth.api.schemas import (
    AuthResponse,
    LoginRequest,
    LogoutRequest,
    RefreshRequest,
    RegisterRequest,
    UserResponse,
)
from app.auth.errors import AuthError
from app.auth.service import AuthService

router = APIRouter(prefix="/api/auth", tags=["auth"])


def _set_auth_cookies(response: Response, session_id: str, refresh_token: str) -> None:
    response.set_cookie(
        key="session_id", value=session_id, httponly=True, samesite="lax"
    )
    response.set_cookie(
        key="refresh_token", value=refresh_token, httponly=True, samesite="lax"
    )


def _clear_auth_cookies(response: Response) -> None:
    response.delete_cookie(key="session_id")
    response.delete_cookie(key="refresh_token")


def _auth_response(tokens) -> AuthResponse:
    return AuthResponse(
        session_id=tokens.session_id,
        refresh_token=tokens.refresh_token,
        username=tokens.user.username,
        role=tokens.user.role,
    )


@router.post("/register", status_code=201, response_model=UserResponse)
async def register(body: RegisterRequest):
    try:
        user = await AuthService.instance().register(body.username, body.password)
    except AuthError as error:
        raise HTTPException(
            status_code=error.status_code, detail=error.detail
        ) from error

    return UserResponse(username=user.username, role=user.role)


@router.post("/login", response_model=AuthResponse)
async def login(body: LoginRequest, response: Response):
    try:
        tokens = await AuthService.instance().login(body.username, body.password)
    except AuthError as error:
        raise HTTPException(
            status_code=error.status_code, detail=error.detail
        ) from error

    _set_auth_cookies(response, tokens.session_id, tokens.refresh_token)
    return _auth_response(tokens)


@router.post("/refresh", response_model=AuthResponse)
async def refresh(body: RefreshRequest, request: Request, response: Response):
    refresh_token = _extract_refresh_token(request, body.refresh_token)
    if refresh_token is None:
        raise HTTPException(status_code=401, detail="Refresh token required")

    try:
        tokens = await AuthService.instance().refresh(refresh_token)
    except AuthError as error:
        raise HTTPException(
            status_code=error.status_code, detail=error.detail
        ) from error

    _set_auth_cookies(response, tokens.session_id, tokens.refresh_token)
    return _auth_response(tokens)


@router.post("/logout", status_code=204)
async def logout(body: LogoutRequest, request: Request, response: Response):
    session_id = _extract_session_id(request)
    refresh_token = _extract_refresh_token(request, body.refresh_token)
    await AuthService.instance().logout(session_id, refresh_token)
    _clear_auth_cookies(response)
    return Response(status_code=204)


@router.get("/me", response_model=UserResponse)
async def me(user: CurrentUser):
    return UserResponse(username=user.username, role=user.role)


@router.delete("/me", status_code=204)
async def delete_me(user: CurrentUser, response: Response):
    try:
        await AuthService.instance().delete_account(user)
    except AuthError as error:
        raise HTTPException(
            status_code=error.status_code, detail=error.detail
        ) from error

    _clear_auth_cookies(response)
    return Response(status_code=204)
