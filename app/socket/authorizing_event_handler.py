from app.socket.session import Session
from app.socket.event_handler import EventHandler

class AuthorizingEventHandler(EventHandler):
    def __init__(self, secket_key: str, inner: EventHandler | None = None):
        self.session: Session | None = None
        self._inner = inner
        self.secket_key = secket_key

    async def start(self, start_message: str) -> None:
        if self.is_closed() or self.session is None:
            print("Cannot start AuthorizingEventHandler: session is closed.")
            return
        
        self.session.send_message(self.secket_key)

        if self._inner: await self._inner.start(start_message)
