import ssl, asyncio

LENGTH_PREFIX_SIZE = 8

class Session(asyncio.Protocol):
    def __init__(self, event_handler):
        self.loop = asyncio.get_event_loop()
        self.buffer = b""
        self.transport = None
        self.expected_len = None
        self.event_handler = event_handler
        self.event_handler.set_session(self)
        self.__tls_upgrading = False
        self.__pending = []

    def connection_made(self, transport):
        self.transport = transport

    def send_message(self, msg: str):
        if self.transport is None:
            print(f"No transport available to send message:\n{msg}")
            return

        if self.__tls_upgrading:
            self.__pending.append(msg)
            return

        if self.transport:
            msg_len = len(msg).to_bytes(LENGTH_PREFIX_SIZE, 'big')
            self.transport.write(msg_len + msg.encode())
    
    def data_received(self, data: bytes):
        self.buffer += data

        while True:
            if self.expected_len is None:
                if len(self.buffer) < LENGTH_PREFIX_SIZE:
                    break
                self.expected_len = int.from_bytes(self.buffer[:LENGTH_PREFIX_SIZE], 'big')
                self.buffer = self.buffer[LENGTH_PREFIX_SIZE:]

            if len(self.buffer) >= self.expected_len:
                msg = self.buffer[:self.expected_len].decode("utf-8")
                asyncio.create_task(self.event_handler.receive_message(msg))
                self.buffer = self.buffer[self.expected_len:]
                self.expected_len = None
            else:
                break

    def connection_lost(self, exc):
        self.transport = None
        self.event_handler.close()
        print("Connection lost with exception: ", exc)

    async def upgrade_to_ssl(self):
        self.__tls_upgrading = True

        ssl_context = ssl.create_default_context()
        ssl_context.check_hostname = False
        ssl_context.verify_mode = ssl.CERT_NONE

        new_transport = await self.loop.start_tls(
            self.transport,
            self,
            ssl_context,
            server_side=False
        )
        self.transport = new_transport
        print("Connection upgraded to SSL")

        self.__tls_upgrading = False
        for msg in self.__pending:
            self.send_message(msg)
        self.__pending.clear()