from fastapi import APIRouter, Request
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from app.services.auth.auth_utils import get_current_user

router = APIRouter(tags=["main"])
templates = Jinja2Templates(directory="templates")

@router.get("/", response_class=HTMLResponse)
async def root(request: Request):
    username: str = await get_current_user(request)

    return templates.TemplateResponse("index.html", {"request": request, "user": username})


@router.get("/dashboard", response_class=HTMLResponse)
async def dashboard(request: Request):
    username: str = await get_current_user(request)
    if username is None:
        return RedirectResponse(url="/login", status_code=302)
    return HTMLResponse(f"<h1>Welcome to the dashboard, {username}!</h1>")
