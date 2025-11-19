from app.socket.connection_protocol import create_connection
import asyncio

class StableConnection:
    def __init__(self, host: str, port: int, event_handler, start_message: str):
        self.host = host
        self.port = port
        self.event_handler = event_handler
        self.init_event_handler = event_handler
        self.start_message = start_message
    
    async def connect(self) -> None:
        print(f"Establishing connection to {self.host}:{self.port}...")
        if not self.event_handler:
            self.event_handler = self.init_event_handler

        self.event_handler = await create_connection(
            host=self.host,
            port=self.port,
            event_handler=self.event_handler,
            start_message=self.start_message)
        
        await self.event_handler.start(start_message=self.start_message)
        
    def close(self) -> None:
        if self.event_handler.session is not None:
            self.event_handler.session.transport.close()
            self.event_handler.session.connection_lost(None)

    async def ensure_connection(self) -> None:
        while not self.event_handler or self.event_handler.is_closed():
            try:
                await self.connect()
            except Exception as e:
                print(f"Failed to initialize connection to {self.host}:{self.port}: {e}")
                try:
                    self.event_handler.close()
                except:
                    self.event_handler = None

            if not self.event_handler or self.event_handler.is_closed():
                await asyncio.sleep(1)
            else:
                print(f"Connection to {self.host}:{self.port} established.")
    
    async def get_connection(self):
        await self.ensure_connection()
        return self.event_handler