from app.socket.event_handler import EventHandler
import asyncio, time

class PingEventHandler(EventHandler):
    def __init__(self, ping_interval: float, pong_timeout: float, inner=None):
        self.session = None
        self._inner = inner
        self.ping_interval = ping_interval
        self.pong_timeout = pong_timeout
        self.last_pong = time.time()

    async def start(self, start_message: str) -> None:
        await super().start(start_message)
        asyncio.create_task(self.ping_loop())

    async def ping_loop(self) -> None:
        await asyncio.sleep(self.ping_interval)

        if self.is_closed():
            return
        
        self.session.send_message("ping")
        await asyncio.sleep(self.pong_timeout)

        if time.time() - self.last_pong > self.pong_timeout + 0.1:
            if self.session is not None:
                self.session.transport.close()
                self.session.connection_lost(Exception("Pong timeout"))
            return
        
        asyncio.create_task(self.ping_loop())

    async def receive_message(self, message: str) -> None:
        if self.is_closed():
            print("Cannot receive message in PingEventHandler: session is closed.")
            return
        
        if (message == "pong"):
            self.last_pong = time.time()
            return
        
        if self._inner: await self._inner.receive_message(message)
