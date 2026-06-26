from oink_judge.pybind11_logger import log_error, log_info, log_warning

from app.services.auth.auth_manager import AuthManager


async def login(username: str, password: str) -> dict:
    session_id = await AuthManager.instance().authenticate(username, password)
    if not session_id:
        log_warning("auth", f"Failed login attempt for user {username}", 3)
        return {"status": "error", "message": "Invalid credentials"}
    log_info("auth", f"User {username} logged in")
    return {"status": "success", "session_id": session_id}


async def whose_session(session_id: str) -> str | None:
    username = await AuthManager.instance().whose_session(session_id)
    if not username:
        return None
    return username


async def register(username: str, password: str) -> bool:
    result = await AuthManager.instance().register_user(username, password)
    if result:
        log_info("auth", f"Registered user {username}")
    else:
        log_warning("auth", f"Registration failed for user {username}")
    return result


async def delete_account(username: str) -> bool:
    result = await AuthManager.instance().delete_user(username)
    if result:
        log_info("auth", f"Deleted account {username}")
    else:
        log_error("auth", f"Failed to delete account {username}")
    return result


async def update_password(username: str, new_password: str) -> bool:
    return await AuthManager.instance().update_password(username, new_password)


async def logout(session_id: str) -> bool:
    await AuthManager.instance().invalidate_session(session_id)
    return True
