from pydantic import BaseModel

from app.auth.roles import Role


class RegisterRequest(BaseModel):
    username: str
    password: str


class LoginRequest(BaseModel):
    username: str
    password: str


class RefreshRequest(BaseModel):
    refresh_token: str


class LogoutRequest(BaseModel):
    refresh_token: str | None = None


class UserResponse(BaseModel):
    username: str
    role: Role


class AuthResponse(BaseModel):
    session_id: str
    refresh_token: str
    username: str
    role: Role
