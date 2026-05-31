from app.config.config import *
from app.socket.authorizing_event_handler import AuthorizingEventHandler
from app.socket.default_event_handler import DefaultEventHandler
from app.socket.ping_event_handler import PingEventHandler
from app.socket.stable_connection import StableConnection

content_service_connection_event_handler = AuthorizingEventHandler(
    Credentials["database"]["password"],
    PingEventHandler(
        Config["ping_interval"], Config["pong_timeout"], DefaultEventHandler()
    ),
)

stable_connection = StableConnection(
    host=Config["hosts"]["content_service"],
    port=Config["ports"]["content_service"],
    event_handler=content_service_connection_event_handler,
    start_message='{"connection_type":"FastAPI"}',
)


async def get_connection():
    global stable_connection
    connection = await stable_connection.get_connection()
    assert connection is not None
    return connection


async def get_manifest_from_server(content_type: str, content_id: str) -> dict:
    connection = await get_connection()

    request = {
        "request": "get_manifest",
        "content_type": content_type,
        content_type + "_id": content_id,
    }
    response = await connection.send_request(request)

    return response["manifest"]


async def download_file_from_server(
    content_type: str, content_id: str, file_path: str
) -> str:
    connection = await get_connection()

    request = {
        "request": "get_file",
        "content_type": content_type,
        content_type + "_id": content_id,
        "file_path": file_path,
    }

    response = await connection.send_request(request)
    import base64

    file_bytes = response["file_content"].encode()
    decoded_from_base64 = base64.b64decode(file_bytes)
    return decoded_from_base64.decode()


async def upload_file_on_server(
    content_type: str, content_id: str, file_path: str, file_content: str
) -> None:
    connection = await get_connection()

    import base64

    file_bytes = file_content.encode()
    encoded_content = base64.b64encode(file_bytes).decode()

    request = {
        "request": "update_file",
        "content_type": content_type,
        content_type + "_id": content_id,
        "file_path": file_path,
        "file_content": encoded_content,
    }

    await connection.send_request(request)


async def delete_file_on_server(
    content_type: str, content_id: str, file_path: str
) -> None:
    connection = await get_connection()

    request = {
        "request": "delete_file",
        "content_type": content_type,
        content_type + "_id": content_id,
        "file_path": file_path,
    }

    await connection.send_request(request)
