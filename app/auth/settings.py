import re
from pathlib import Path

from oink_judge.pybind11_config import get_token_from_credentials

from app.config import check_object_is_number_integer, check_object_is_string, config

DEFAULT_CONFIG_PATH = Path("configs/app/config.json")
DEFAULT_USERNAME_REGEX = r"^[a-zA-Z0-9.\-_?!()]{3,100}$"
DEFAULT_SESSION_TTL_SECONDS = 900
DEFAULT_REFRESH_TOKEN_TTL_SECONDS = 604800


def get_username_regex() -> re.Pattern[str]:
    if not check_object_is_string(config(), ["auth", "username_regex"]):
        return re.compile(DEFAULT_USERNAME_REGEX)
    return re.compile(config()["auth"]["username_regex"])


def get_session_ttl_seconds() -> int:
    if not check_object_is_number_integer(config(), ["auth", "session_ttl_seconds"]):
        return DEFAULT_SESSION_TTL_SECONDS
    return config()["auth"]["session_ttl_seconds"]


def get_refresh_token_ttl_seconds() -> int:
    if not check_object_is_number_integer(
        config(), ["auth", "refresh_token_ttl_seconds"]
    ):
        return DEFAULT_REFRESH_TOKEN_TTL_SECONDS
    return config()["auth"]["refresh_token_ttl_seconds"]


def get_redis_host() -> str:
    if not check_object_is_string(config(), ["redis", "host"]):
        return "127.0.0.1"
    return config()["redis"]["host"]


def get_redis_port() -> int:
    if not check_object_is_number_integer(config(), ["redis", "port"]):
        return 6379
    return config()["redis"]["port"]


def get_redis_password() -> str | None:
    password = get_token_from_credentials("redis.password")
    if password is None or password == "":
        return None
    return password
