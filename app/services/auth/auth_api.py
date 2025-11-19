from app.socket.stable_connection import StableConnection
from app.socket.authorizing_event_handler import AuthorizingEventHandler
from app.socket.ping_event_handler import PingEventHandler
from app.socket.default_event_handler import DefaultEventHandler
from app.config.config import *


auth_connection_event_handler = AuthorizingEventHandler(Credentials["database"]["password"], 
                                    PingEventHandler(Config["ping_interval"], Config["pong_timeout"], 
                                    DefaultEventHandler()))

stable_connection = StableConnection(
    host=Config["hosts"]["auth"],
    port=Config["ports"]["auth"],
    event_handler=auth_connection_event_handler,
    start_message='{"connection_type":"FastAPI"}'
)

async def get_connection():
    global stable_connection
    connection = await stable_connection.get_connection()
    return connection



async def login(username: str, password: str) -> dict:
    connection = await get_connection()

    request = {
        "request": "login",
        "username": username,
        "password": password
    }
    response = await connection.send_request(request)
    
    return response



async def whose_session(session_id: str) -> str | None:
    connection = await get_connection()
    
    request = {
        "request": "whose_session",
        "session_id": session_id
    }
    response = await connection.send_request(request)
    if response["status"] == "success":
        return response["username"]
    else:
        return None



async def register(username: str, password: str) -> bool:
    connection = await get_connection()

    request = {
        "request": "register",
        "username": username,
        "password": password
    }
    response = await connection.send_request(request)
    return response["status"] == "success"



async def delete_account(username: str) -> bool:
    connection = await get_connection()

    request = {
        "request": "delete_user",
        "username": username
    }
    response = await connection.send_request(request)
    return response["status"] == "success"



async def update_password(username: str, new_password: str) -> bool:
    connection = await get_connection()

    request = {
        "request": "update_password",
        "username": username,
        "new_password": new_password
    }
    response = await connection.send_request(request)
    return response["status"] == "success"



async def logout(session_id: str) -> bool:
    connection = await get_connection()

    request = {
        "request": "logout",
        "session_id": session_id
    }
    response = await connection.send_request(request)
    return response["status"] == "success"
