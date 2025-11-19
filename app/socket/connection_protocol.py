from app.socket.session import Session
import asyncio

async def create_connection(host: str, port: int, event_handler, start_message: str):
    loop = asyncio.get_event_loop()
    protocol_factory = lambda: Session(event_handler)
    transport, protocol = await loop.create_connection(
        protocol_factory,
        host,
        port
    )
    
    protocol.send_message(start_message)
    
    await protocol.upgrade_to_ssl()

    return protocol.event_handler
