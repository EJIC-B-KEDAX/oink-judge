import app.services.auth.auth_api as auth_api
from fastapi import Request

async def get_current_user(request: Request) -> str | None:
    session_id = request.cookies.get("session_id")
    if session_id is None:
        return None
    username = await auth_api.whose_session(session_id)
    return username