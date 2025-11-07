from app.socket.session import Session
import asyncio, time

class AuthorizingEventHandler:
    def __init__(self, secket_key: str, inner=None):
        self.session = None
        self.__inner = inner
        self.secket_key = secket_key

    def set_session(self, session: Session) -> None:
        self.session = session
        if self.__inner: self.__inner.set_session(session)

    async def start(self, start_message: str) -> None:
        await self.session.send(self.secket_key)
        if self.__inner: await self.__inner.start(start_message)

    async def receive_message(self, message: str) -> None:
        if self.__inner: await self.__inner.receive_message(message)

    async def send_request(self, request: dict) -> dict:
        if self.__inner: return await self.__inner.send_request(request)

        raise RuntimeError("Inner event handler is not set for PongEventHandler.")
