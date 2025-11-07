from app.socket.session import Session
import asyncio, time

class PingEventHandler:
    def __init__(self, ping_interval: float, pong_timeout: float, inner=None):
        self.session = None
        self.__inner = inner
        self.ping_interval = ping_interval
        self.pong_timeout = pong_timeout
        self.last_pong = time.time()

    def set_session(self, session: Session) -> None:
        self.session = session
        if self.__inner: self.__inner.set_session(session)

    async def start(self, start_message: str) -> None:
        asyncio.create_task(self.ping_loop())
        if self.__inner: await self.__inner.start(start_message)

    async def ping_loop(self) -> None:
        await asyncio.sleep(self.ping_interval)
        await self.session.send("ping")
        print("Ping sent.")
        await asyncio.sleep(self.pong_timeout)
        if time.time() - self.last_pong > self.pong_timeout + 0.1:
            print(time.time() - self.last_pong, self.pong_timeout)
            print("Pong timeout exceeded. Closing session.")
            await self.session.close()
            self.session = None
            return
        asyncio.create_task(self.ping_loop())

    async def receive_message(self, message: str) -> None:
        print(f"PingEventHandler received message: {message}")
        if (message == "pong"):
            self.last_pong = time.time()
            asyncio.create_task(self.session.receive())
            return
        if self.__inner: await self.__inner.receive_message(message)

    async def send_request(self, request: dict) -> dict:
        if self.__inner: return await self.__inner.send_request(request)

        raise RuntimeError("Inner event handler is not set for PongEventHandler.")
