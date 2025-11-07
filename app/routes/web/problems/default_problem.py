from fastapi import APIRouter, Request, Form, UploadFile, Depends
from fastapi.responses import HTMLResponse, RedirectResponse, PlainTextResponse, FileResponse
from fastapi.templating import Jinja2Templates
from app.config.problem_config import get_problem_statements
from app.services.auth.auth_utils import get_current_user
from app.database.tables.submissions import add_submission, SubmissionInfo, load_submissions_by_user_and_problem
from app.database.database import get_db
from app.services.dispatcher.dispatcher_api import handle_submission
from datetime import datetime
import uuid, os


__type_name__ = "dp"

router = APIRouter(prefix=f"/problems/{__type_name__}", tags=["default_problem"])
templates = Jinja2Templates(directory="templates/problems")


class ProblemInfo:
    def __init__(self, problem_id: str, type_name: str, html_statements: str | None = None):
        self.id = problem_id
        self.type = type_name
        self.html_statements = html_statements



@router.get("/{problem_id}", response_class=HTMLResponse)
async def problem_statements(problem_id: str, request: Request):
    statements = get_problem_statements(problem_id, "english", "text/html")

    if statements is None:
        statements = "<p>Problem statements are not available.</p>"

    problem_info = ProblemInfo(
        problem_id=problem_id,
        type_name=__type_name__,
        html_statements=statements
    )

    return templates.TemplateResponse("default_statements.html", {
        "request": request,
        "problem": problem_info
    })



@router.get("/{problem_id}/problem-statement.css", response_class=HTMLResponse)
async def problem_statement_css(problem_id: str):
    css_path = os.path.join("problems", problem_id, "statements", ".html", "english", "problem-statement.css")
    return FileResponse(css_path, media_type="text/css")



@router.get("/{problem_id}/submit", response_class=HTMLResponse)
async def problem_submit_page(problem_id: str, request: Request):
    problem_info = ProblemInfo(
        problem_id=problem_id,
        type_name=__type_name__
    )

    return templates.TemplateResponse("default_submit.html", {
        "request": request,
        "problem": problem_info
    })



@router.post("/{problem_id}/submit", response_class=HTMLResponse)
async def problem_submit(problem_id: str, request: Request,
        language: str = Form(...),
        solution: UploadFile = Form(...),
        db = Depends(get_db)):

    username: str = await get_current_user(request)
    if username is None:
        return RedirectResponse(url="/login", status_code=302)

    submission_id = str(uuid.uuid4())

    submission_dir = f"submissions/{submission_id}"
    os.makedirs(submission_dir, exist_ok=True)

    source_path = os.path.join(submission_dir, "source.cpp")
    with open(source_path, "wb") as f:
        f.write(await solution.read())

    submission_info = SubmissionInfo(
        id=submission_id,
        username=username,
        problem_id=problem_id,
        language=language,
        verdict_type="TS",
        score=0.0,
        send_time=datetime.now().replace(microsecond=0)
    )

    await add_submission(db, submission_info)

    await handle_submission(submission_id)

    return RedirectResponse(url=f"/problems/{__type_name__}/{problem_id}/submissions", status_code=302)
    


@router.get("/{problem_id}/submissions", response_class=HTMLResponse)
async def problem_submissions(problem_id: str, request: Request, db = Depends(get_db)):
    username: str = await get_current_user(request)
    if username is None:
        return RedirectResponse(url="/login", status_code=302)

    submissions = await load_submissions_by_user_and_problem(db, username, problem_id)
    problem_info = ProblemInfo(
        problem_id=problem_id,
        type_name=__type_name__
    )

    return templates.TemplateResponse("default_submissions.html", {
        "request": request,
        "problem": problem_info,
        "submissions": submissions
    })