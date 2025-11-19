from app.socket.stable_connection import StableConnection
from app.socket.authorizing_event_handler import AuthorizingEventHandler
from app.socket.ping_event_handler import PingEventHandler
from app.socket.default_event_handler import DefaultEventHandler
from app.config.config import *
import asyncio

dispatcher_connection_event_handler = AuthorizingEventHandler(Credentials["database"]["password"], 
                                    PingEventHandler(Config["ping_interval"], Config["pong_timeout"], 
                                    DefaultEventHandler()))

stable_connection = StableConnection(
    host=Config["hosts"]["dispatcher"],
    port=Config["ports"]["dispatcher"],
    event_handler=dispatcher_connection_event_handler,
    start_message='{"connection_type":"FastAPI"}'
)

async def handle_submission(submission_id: str) -> bool:
    global stable_connection
    connection = await stable_connection.get_connection()
    
    request = {
        "request": "handle_submission",
        "submission_id": submission_id
    }
    response = await connection.send_request(request)
    print("Dispatcher response:", response)
    return response["status"] == "success"
