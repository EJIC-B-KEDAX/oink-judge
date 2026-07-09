import json
import re
from functools import lru_cache
from pathlib import Path

from oink_judge.pybind11_config import get_token_from_credentials

DEFAULT_CONFIG_PATH = Path("configs/app/config.json")
DEFAULT_USERNAME_REGEX = r"^[a-zA-Z0-9.\-_?!()]{3,100}$"
DEFAULT_SESSION_TTL_SECONDS = 900
DEFAULT_REFRESH_TOKEN_TTL_SECONDS = 604800


@lru_cache(maxsize=1)
def _load_config() -> dict:
    if not DEFAULT_CONFIG_PATH.exists():
        return {}
    with DEFAULT_CONFIG_PATH.open(encoding="utf-8") as config_file:
        return json.load(config_file)


def get_username_regex() -> re.Pattern[str]:
    auth_config = _load_config().get("auth", {})
    pattern = auth_config.get("username_regex", DEFAULT_USERNAME_REGEX)
    return re.compile(pattern)


def get_session_ttl_seconds() -> int:
    auth_config = _load_config().get("auth", {})
    return int(auth_config.get("session_ttl_seconds", DEFAULT_SESSION_TTL_SECONDS))


def get_refresh_token_ttl_seconds() -> int:
    auth_config = _load_config().get("auth", {})
    return int(
        auth_config.get("refresh_token_ttl_seconds", DEFAULT_REFRESH_TOKEN_TTL_SECONDS)
    )


def get_redis_host() -> str:
    redis_config = _load_config().get("redis", {})
    return str(redis_config.get("host", "127.0.0.1"))


def get_redis_port() -> int:
    redis_config = _load_config().get("redis", {})
    return int(redis_config.get("port", 6379))


def get_redis_password() -> str | None:
    password = get_token_from_credentials("redis.password")
    if password is None or password == "":
        return None
    return password
