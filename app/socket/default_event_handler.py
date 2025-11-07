import asyncio, uuid, json
from app.socket.session import Session

class DefaultEventHandler:
    def __init__(self):
        self.session = None
        self.pending = {}

    def set_session(self, session: Session) -> None:
        self.session = session
    
    async def start(self, start_message: str) -> None:
        asyncio.create_task(self.session.receive())

    async def receive_message(self, message: str) -> None:
        data = json.loads(message)
        if '__id__' in data and data['__id__'] in self.pending:
            future = self.pending.pop(data['__id__'])
            future.set_result(data)
        asyncio.create_task(self.session.receive())
    
    async def send_request(self, request: dict) -> dict:
        if not self.session:
            raise RuntimeError("Session is not set for the event handler.")
        
        request_id = str(uuid.uuid4())
        request['__id__'] = request_id
        future = asyncio.get_event_loop().create_future()
        self.pending[request_id] = future
        await self.session.send(json.dumps(request))

        return await future
