import json
import socket
from config import config

def ask(service: str, data: dict) -> dict:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        port = config.get_port(service)

        sock.connect(("127.0.0.1", port))
        request_data = json.dumps(data).encode('utf-8')

        start_message = '{"connection_type":"FastAPI"}'

        sock.sendall(len(start_message).to_bytes(8, 'big'))
        print("Sending start message:", start_message)
        sock.sendall(start_message.encode('utf-8'))

        sock.sendall(len(request_data).to_bytes(8, 'big'))
        sock.sendall(request_data)
        
        response_length = int.from_bytes(sock.recv(8), 'big')

        response = b''
        while response_length > len(response):
            part = sock.recv(response_length - len(response))
            response += part
        
        return json.loads(response.decode('utf-8'))
    except Exception as e:
        return {"error": str(e)}
    finally:
        sock.close()

def ask_no_answer(service: str, data: dict):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        port = config.get_port(service)

        sock.connect(("127.0.0.1", port))
        request_data = json.dumps(data).encode('utf-8')

        start_message = '{"connection_type":"FastAPI"}'

        sock.sendall(len(start_message).to_bytes(8, 'big'))
        print("Sending start message:", start_message)
        sock.sendall(start_message.encode('utf-8'))

        sock.sendall(len(request_data).to_bytes(8, 'big'))
        sock.sendall(request_data)
        
    except Exception as e:
        print(str(e))
    finally:
        sock.close()


def get_session_id(username: str) -> str:
    data = {"request": "get_session_id", "username": username}
    response = ask("auth", data)
    if "error" in response:
        raise Exception(f"Error getting session ID: {response['error']}")
    return response["session_id"]

def whose_session(session_id: str) -> str:
    data = {"request": "whose_session", "session_id": session_id}
    response = ask("auth", data)
    if "error" in response:
        raise Exception(f"Error getting session owner: {response['error']}")
    return response["username"]

def handle_submission(submission_id: str):
    data = {"request_type": "handle_submission", "submission_id": submission_id}
    ask_no_answer("management", data)
    return

def get_score(username: str, problem_id: str):
    data = {"type": "get_score", "username": username, "problem_id": problem_id}
    response = ask("management", data)
    print(response)
    return response["score"]