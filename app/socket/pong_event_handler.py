from app.socket.event_handler import EventHandler

class PongEventHandler(EventHandler):
    async def receive_message(self, message: str) -> None:
        if self.is_closed():
            print("Cannot receive message in PongEventHandler: session is closed.")
            return
        
        if (message == "ping"):
            self.session.send_message("pong")
            return
        
        if self._inner: await self._inner.receive_message(message)
