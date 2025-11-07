from app.socket.session import Session
from app.socket.authorizing_event_handler import AuthorizingEventHandler
from app.socket.ping_event_handler import PingEventHandler
from app.socket.default_event_handler import DefaultEventHandler
from app.config.config import *


dispatcher_connection_event_handler = None
dispatcher_connection = None

async def initialize_dispatcher_connection():
    global dispatcher_connection_event_handler, dispatcher_connection

    dispatcher_connection_event_handler = AuthorizingEventHandler(Credentials["database"]["password"], 
                                        PingEventHandler(Config["ping_interval"], Config["pong_timeout"], 
                                        DefaultEventHandler()))

    dispatcher_connection = Session(
        host=Config["hosts"]["dispatcher"],
        port=Config["ports"]["dispatcher"],
        event_handler=dispatcher_connection_event_handler,
        use_ssl=Config["use_ssl"]["dispatcher"])

    dispatcher_connection_event_handler.set_session(dispatcher_connection)

    await dispatcher_connection.connect()

    await dispatcher_connection.send('{"connection_type":"FastAPI"}')

    await dispatcher_connection_event_handler.start("")



async def close_dispatcher_connection():
    global dispatcher_connection_event_handler, dispatcher_connection

    if dispatcher_connection:
        await dispatcher_connection.close()
        dispatcher_connection = None
        dispatcher_connection_event_handler = None



async def ensure_dispatcher_connection():
    global dispatcher_connection_event_handler

    if not dispatcher_connection_event_handler or not dispatcher_connection_event_handler.session:
        await initialize_dispatcher_connection()



async def handle_submission(submission_id: str) -> bool:
    global dispatcher_connection_event_handler
    
    await ensure_dispatcher_connection()
    
    request = {
        "request": "handle_submission",
        "submission_id": submission_id
    }
    response = await dispatcher_connection_event_handler.send_request(request)
    return response["status"] == "success"
