from fastapi import APIRouter, Request
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from app.services.auth.auth_utils import get_current_user
from pathlib import Path

router = APIRouter(tags=["main"])
templates = Jinja2Templates(directory="templates")

PROBLEMS_DIR = Path("problems")

@router.get("/", response_class=HTMLResponse)
async def root(request: Request):
    username: str = await get_current_user(request)

    return templates.TemplateResponse("index.html", {"request": request, "user": username})


@router.get("/dashboard", response_class=HTMLResponse)
async def dashboard(request: Request):
    problems = []

    if PROBLEMS_DIR.exists():
        for p in sorted(PROBLEMS_DIR.iterdir()):
            if p.is_dir() and not p.name.startswith("."):
                problems.append(p.name)

    return templates.TemplateResponse(
        "dashboard.html",
        {
            "request": request,
            "problems": problems,
        }
    )