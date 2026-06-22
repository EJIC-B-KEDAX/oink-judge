import hashlib
import secrets
import time

from app.config import get_timing, require_has_value


class Session:
    def __init__(self, username: str) -> None:
        self._username = username
        self._session_id = ""
        self._expire_at = 0

    def generate_session(self) -> None:
        if self._session_id:
            return
        self._session_id = self._generate_session_id()
        self._expire_at = self._generate_expired_at()

    @property
    def session_id(self) -> str:
        if not self.is_valid:
            return ""
        return self._session_id

    @property
    def username(self) -> str:
        return self._username

    @property
    def expire_at(self) -> int:
        return self._expire_at

    @property
    def is_valid(self) -> bool:
        return time.time() <= self._expire_at

    def _generate_session_id(self) -> str:
        data = f"{int(time.time())}.{secrets.randbelow(2**64)}.{self._username}"
        return hashlib.sha256(data.encode()).hexdigest()

    def _generate_expired_at(self) -> int:
        timing = get_timing("valid_session_time")
        if timing is None:
            seconds = 3600
        else:
            seconds = int(require_has_value(timing).total_seconds())
        return int(time.time()) + seconds
