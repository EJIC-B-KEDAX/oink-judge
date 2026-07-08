from app.auth.settings import get_username_regex


def validate_username(username: str) -> str | None:
    if get_username_regex().fullmatch(username) is None:
        return "Invalid username format"
    return None
