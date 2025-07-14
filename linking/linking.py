import time
import re
from fastapi import FastAPI, Form, Request, UploadFile
from fastapi.responses import HTMLResponse, RedirectResponse, PlainTextResponse
from fastapi.templating import Jinja2Templates
from submission import *
from config import config
import requests_to_server
import os
import uuid

app = FastAPI()
templates = Jinja2Templates(directory=config.get_directory("templates"))

async def get_current_user(request: Request) -> str | None:
    session_id: str = request.cookies.get("session_id")
    if session_id is None:
        return None

    try:
        username = requests_to_server.whose_session(session_id)
    except Exception:
        return None
    
    if username == "":
        return None
    return username

@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    username: str = await get_current_user(request)

    return templates.TemplateResponse("index.html", {"request": request, "user": username})

@app.get("/login", response_class=HTMLResponse)
async def show_login_page(request: Request):
    if await get_current_user(request) is not None:
        return RedirectResponse(url="/dashboard", status_code=302)

    return templates.TemplateResponse("login.html", {"request": request})

@app.get("/logout")
async def handle_logout(request: Request):
    session_id: str = request.cookies.get("session_id")
    if session_id is not None:
        requests_to_server.ask("auth", {"request": "logout", "session_id": session_id})

    response = RedirectResponse(url="/login", status_code=302)
    response.delete_cookie("session_id")
    return response

@app.get("/register", response_class=HTMLResponse)
async def show_register_page(request: Request):
    if await get_current_user(request) is not None:
        return RedirectResponse(url="/dashboard", status_code=302)

    return templates.TemplateResponse("register.html", {"request": request})

@app.post("/register", response_class=HTMLResponse, name="register")
async def handle_register(request: Request, username: str = Form(...), password: str = Form(...), confirm_password: str = Form(...)):
    if not re.fullmatch(r"[a-zA-Z0-9.\-_?!()]{3,100}", username):
        return templates.TemplateResponse("register.html", {"request": request, "error": "invalid characters in username, check pattern is [a-zA-Z0-9.-_?!()]{3,100}"})

    if confirm_password != password:
        return templates.TemplateResponse("register.html", {"request": request, "error": "passwords do not match"})

    auth_response = requests_to_server.ask("auth", {"request": "register", "username": username, "password": password})

    if "error" in auth_response:
        return templates.TemplateResponse("register.html", {"request": request, "error": auth_response["error"]})

    if auth_response.get("status") == "success":
        return RedirectResponse(url="/login", status_code=302)
    
    return templates.TemplateResponse("register.html", {"request": request, "error": "Registration failed"})

@app.get("/delete_account", response_class=HTMLResponse)
async def show_delete_account_page(request: Request):
    username: str = await get_current_user(request)
    if username is None:
        return RedirectResponse(url="/login", status_code=302)

    return templates.TemplateResponse("delete_account.html", {"request": request, "username": username})

@app.post("/delete_account", response_class=HTMLResponse, name="delete_account")
async def handle_delete_account(request: Request):
    username: str = await get_current_user(request)
    if username is None:
        return RedirectResponse(url="/login", status_code=302)

    auth_response = requests_to_server.ask("auth", {"request": "delete_user", "username": username})

    if "error" in auth_response:
        return templates.TemplateResponse("delete_account.html", {"request": request, "error": auth_response["error"]})

    if auth_response.get("status") == "success":
        response = RedirectResponse(url="/login", status_code=302)
        response.delete_cookie("session_id")
        return response
    
    return templates.TemplateResponse("delete_account.html", {"request": request, "error": "Account deletion failed"})

@app.post("/login", name="login")
async def handle_login(request: Request, username: str = Form(...), password: str = Form(...)):
    auth_response = requests_to_server.ask("auth", {"request": "login", "username": username, "password": password})

    if "error" in auth_response:
        return templates.TemplateResponse("login.html", {"request": request, "error": auth_response["error"]})

    if auth_response.get("status") == "success":
        session_id = auth_response["session_id"]
        response = RedirectResponse(url="/dashboard", status_code=302)
        response.set_cookie(key="session_id", value=session_id, httponly=True)
        return response
    return templates.TemplateResponse("login.html", {"request": request, "error": "Invalid credentials"})

@app.get("/dashboard", response_class=HTMLResponse)
async def dashboard(request: Request):
    username: str = await get_current_user(request)
    if username is None:
        return RedirectResponse(url="/login", status_code=302)
    return HTMLResponse(f"<h1>Welcome to the dashboard, {username}!</h1>")

@app.get("/submit", response_class=HTMLResponse)
async def dashboard(request: Request):
    username: str = await get_current_user(request)
    if username is None:
        return RedirectResponse(url="/login", status_code=302)
    return templates.TemplateResponse("submit.html", {"request": request})

@app.post("/api/submit")
async def submit_solution(request: Request,
        problem_id: str = Form(...),
        language: str = Form(...),
        solution: UploadFile = Form(...)
):
    username: str = await get_current_user(request)
    if username is None:
        return RedirectResponse(url="/login", status_code=302)

    submission_id = str(uuid.uuid4())

    submission_dir = f"../submissions/{submission_id}"
    os.makedirs(submission_dir, exist_ok=True)

    source_path = os.path.join(submission_dir, "source.cpp")
    with open(source_path, "wb") as f:
        f.write(await solution.read())

    store_submission_info(submission_id, SubmissionInfo(username, language, problem_id, int(time.time())))

    requests_to_server.handle_submission(submission_id)
    score = requests_to_server.get_score(username, problem_id)

    return PlainTextResponse(str(score))