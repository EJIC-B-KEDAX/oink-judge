from fastapi import APIRouter, Depends, Request, Form
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from app.services.auth.auth_utils import get_current_user, require_current_user
from app.services.auth.auth_api import *
import re

router = APIRouter(tags=["auth"])
templates = Jinja2Templates(directory="templates/auth")



@router.get("/login", response_class=HTMLResponse)
async def show_login_page(request: Request, username: str = Depends(get_current_user)):
    if username is not None:
        return RedirectResponse(url="/dashboard", status_code=302)

    return templates.TemplateResponse("login.html", {"request": request})



@router.get("/logout", response_class=HTMLResponse)
async def handle_logout(request: Request):
    session_id: str = request.cookies.get("session_id")
    if session_id is not None:
        await logout(session_id)

    response = RedirectResponse(url="/login", status_code=302)
    response.delete_cookie("session_id")
    return response



@router.get("/register", response_class=HTMLResponse)
async def show_register_page(request: Request, username: str = Depends(get_current_user)):
    if await username is not None:
        return RedirectResponse(url="/dashboard", status_code=302)

    return templates.TemplateResponse("register.html", {"request": request})



@router.post("/register", response_class=HTMLResponse, name="register")
async def handle_register(request: Request, username: str = Form(...), password: str = Form(...), confirm_password: str = Form(...)):
    if not re.fullmatch(r"[a-zA-Z0-9.\-_?!()]{3,100}", username):
        return templates.TemplateResponse("register.html", {"request": request, "error": "invalid characters in username, check pattern is [a-zA-Z0-9.-_?!()]{3,100}"})

    if confirm_password != password:
        return templates.TemplateResponse("register.html", {"request": request, "error": "passwords do not match"})

    auth_response = await register(username, password)

    if auth_response == True:
        return RedirectResponse(url="/login", status_code=302)
    
    return templates.TemplateResponse("register.html", {"request": request, "error": "Registration failed"})



@router.get("/delete_account", response_class=HTMLResponse)
async def show_delete_account_page(request: Request, username: str = Depends(require_current_user)):
    return templates.TemplateResponse("delete_account.html", {"request": request, "username": username})



@router.post("/delete_account", response_class=HTMLResponse, name="delete_account")
async def handle_delete_account(request: Request, username: str = Depends(require_current_user)):
    auth_response = delete_account(username)

    if auth_response == True:
        response = RedirectResponse(url="/login", status_code=302)
        response.delete_cookie("session_id")
        return response
    
    return templates.TemplateResponse("delete_account.html", {"request": request, "error": "Account deletion failed"})



@router.post("/login", name="login")
async def handle_login(request: Request, username: str = Form(...), password: str = Form(...)):
    auth_response = await login(username, password)

    if auth_response.get("status") == "error":
        return templates.TemplateResponse("login.html", {"request": request, "error": auth_response["message"]})
    else:
        session_id = auth_response["session_id"]
        response = RedirectResponse(url="/dashboard", status_code=302)
        response.set_cookie(key="session_id", value=session_id, httponly=True)
        return response
