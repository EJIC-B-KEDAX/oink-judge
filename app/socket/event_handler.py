from app.socket.session import Session

class EventHandler:
    def __init__(self, inner=None):
        self.session = None
        self._inner = inner

    def set_session(self, session: Session) -> None:
        self.session = session
        if self._inner: self._inner.set_session(session)

    def is_closed(self) -> bool:
        if self._inner:
            return self.session is None or self._inner.is_closed()
        
        return self.session is None

    async def start(self, start_message: str) -> None:
        if self.is_closed():
            print("Cannot start EventHandler: session is closed.")
            return

        if self._inner: await self._inner.start(start_message)

    async def receive_message(self, message: str) -> None:
        if self.is_closed():
            print("Cannot receive message in EventHandler: session is closed.")
            return
        
        if self._inner: await self._inner.receive_message(message)

    async def send_request(self, request: dict) -> dict:
        if self.is_closed():
            return {"status": "error", "message": "Cannot send request: session is closed."}
        
        if self._inner: return await self._inner.send_request(request)

        return {"status": "error", "message": "Inner event handler is not set for EventHandler."}
    
    def close(self) -> None:
        self.session = None
        if self._inner: self._inner.close()
