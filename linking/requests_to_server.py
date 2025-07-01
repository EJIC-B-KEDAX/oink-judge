import json
import socket
import config

def ask(service: str, data: dict) -> dict:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        port = config.AUTH_PORT
        if service == "management":
            port = config.MANAGEMENT_PORT

        sock.connect(("127.0.0.1", port))
        request_data = json.dumps(data).encode('utf-8')

        sock.sendall(len(request_data).to_bytes(4, 'big'))
        sock.sendall(request_data)
        
        response_length = int.from_bytes(sock.recv(4), 'big')

        response = b''
        while response_length > len(response):
            part = sock.recv(response_length - len(response))
            response += part
        
        return json.loads(response.decode('utf-8'))
    except Exception as e:
        return {"error": str(e)}
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
    data = {"type": "test_submission", "submission_id": submission_id}
    ask("management", data)
    return

def get_score(username: str, problem_id: str):
    data = {"type": "get_score", "username": username, "problem_id": problem_id}
    response = ask("management", data)
    print(response)
    return response["score"]