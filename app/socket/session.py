import ssl, asyncio

class Session:
    def __init__(self, host: str, port: int, event_handler, use_ssl=False, ssl_context=None):
        self.host = host
        self.port = port
        self.use_ssl = use_ssl
        self.ssl_context = ssl_context or (ssl.create_default_context() if use_ssl else None)
        self.reader = None
        self.writer = None
        self.__event_handler = event_handler

    async def connect(self) -> None:
        if self.use_ssl:
            self.reader, self.writer = await asyncio.open_connection(
                self.host, self.port, ssl=self.ssl_context
            )
        else:
            self.reader, self.writer = await asyncio.open_connection(
                self.host, self.port
            )

    async def send(self, data: str) -> None:
        if not self.writer:
            raise ConnectionError("Not connected to the server.")
        
        self.writer.write(len(data).to_bytes(8, 'big'))
        self.writer.write(data.encode())
        
        await self.writer.drain()

    async def receive(self) -> None:
        if not self.reader:
            raise ConnectionError("Not connected to the server.")
        
        length_data = int.from_bytes(await self.reader.readexactly(8), 'big')
        data = await self.reader.readexactly(length_data)

        asyncio.create_task(self.__event_handler.receive_message(data.decode('utf-8')))

    async def close(self) -> None:
        if self.writer:
            self.writer.close()
            await self.writer.wait_closed()
            self.reader = None
            self.writer = None