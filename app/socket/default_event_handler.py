from app.socket.event_handler import EventHandler
import asyncio, uuid, json

class DefaultEventHandler(EventHandler):
    def __init__(self):
        self.session = None
        self._inner = None
        self.pending = {}

    async def receive_message(self, message: str) -> None:
        if self.is_closed():
            print("Cannot receive message in DefaultEventHandler: session is closed.")
            return
        
        data = json.loads(message)
        if '__id__' in data and data['__id__'] in self.pending:
            future = self.pending.pop(data['__id__'])
            future.set_result(data)
    
    async def send_request(self, request: dict) -> dict:
        if self.is_closed():
            return {"status": "error", "message": "Cannot send request: session is closed."}
        
        request_id = str(uuid.uuid4())
        request['__id__'] = request_id
        future = asyncio.get_event_loop().create_future()
        self.pending[request_id] = future
        self.session.send_message(json.dumps(request))

        return await future
    
    def close(self) -> None:
        for future in self.pending.values():
            if not future.done():
                future.set_result({"status": "error", "message": "Connection closed."})

        self.pending.clear()
        self.session = None
