from app.socket.session import Session
from app.socket.authorizing_event_handler import AuthorizingEventHandler
from app.socket.ping_event_handler import PingEventHandler
from app.socket.default_event_handler import DefaultEventHandler
from app.config.config import *

auth_connection_event_handler = None
auth_connection = None

async def initialize_auth_connection():
    global auth_connection_event_handler, auth_connection

    auth_connection_event_handler = AuthorizingEventHandler(Credentials["database"]["password"], 
                                        PingEventHandler(Config["ping_interval"], Config["pong_timeout"], 
                                        DefaultEventHandler()))

    auth_connection = Session(
        host=Config["hosts"]["auth"],
        port=Config["ports"]["auth"],
        event_handler=auth_connection_event_handler,
        use_ssl=Config["use_ssl"]["auth"])

    auth_connection_event_handler.set_session(auth_connection)

    await auth_connection.connect()

    await auth_connection.send('{"connection_type":"FastAPI"}')

    await auth_connection_event_handler.start("")



async def close_auth_connection():
    global auth_connection_event_handler, auth_connection

    if auth_connection:
        await auth_connection.close()
        auth_connection = None
        auth_connection_event_handler = None



async def ensure_auth_connection():
    global auth_connection_event_handler

    if not auth_connection_event_handler or not auth_connection_event_handler.session:
        await initialize_auth_connection()



async def login(username: str, password: str) -> dict:
    global auth_connection_event_handler

    await ensure_auth_connection()

    request = {
        "request": "login",
        "username": username,
        "password": password
    }
    response = await auth_connection_event_handler.send_request(request)
    
    if response["status"] == "success":
        return {
            "status": "success",
            "session_id": response["session_id"]
            }
    else:
        return {
            "status": "error",
            "message": response["message"]
            }



async def whose_session(session_id: str) -> str | None:
    global auth_connection_event_handler
    
    await ensure_auth_connection()
    
    request = {
        "request": "whose_session",
        "session_id": session_id
    }
    response = await auth_connection_event_handler.send_request(request)
    if response["status"] == "success":
        return response["username"]
    else:
        return None



async def register(username: str, password: str) -> bool:
    global auth_connection_event_handler

    await ensure_auth_connection()

    request = {
        "request": "register",
        "username": username,
        "password": password
    }
    response = await auth_connection_event_handler.send_request(request)
    return response["status"] == "success"



async def delete_account(username: str) -> bool:
    global auth_connection_event_handler

    await ensure_auth_connection()

    request = {
        "request": "delete_user",
        "username": username
    }
    response = await auth_connection_event_handler.send_request(request)
    return response["status"] == "success"



async def update_password(username: str, new_password: str) -> bool:
    global auth_connection_event_handler

    await ensure_auth_connection()

    request = {
        "request": "update_password",
        "username": username,
        "new_password": new_password
    }
    response = await auth_connection_event_handler.send_request(request)
    return response["status"] == "success"



async def logout(session_id: str) -> bool:
    global auth_connection_event_handler

    await ensure_auth_connection()

    request = {
        "request": "logout",
        "session_id": session_id
    }
    response = await auth_connection_event_handler.send_request(request)
    return response["status"] == "success"
