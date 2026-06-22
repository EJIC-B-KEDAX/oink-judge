import os
import uuid
from datetime import datetime

from fastapi import APIRouter, Depends, Form, Request, UploadFile
from fastapi.responses import FileResponse, HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates

from app.config import get_problem_statements
from app.database import AsyncTableSubmissions, SubmissionRow
from app.services.auth.auth_utils import require_current_user
from app.services.dispatcher.dispatcher_api import handle_submission
from oink_judge.pybind11_logger import log_error, log_info

templates = Jinja2Templates(directory="templates/problems")


class ProblemInfo:
    def __init__(self, problem_id: str, html_statements: str | None = None):
        self.id = problem_id
        self.html_statements = html_statements


class OpenProblem:
    def __init__(self, problem_id: str):
        self.id = problem_id
        self.router = APIRouter(
            prefix=f"/problems/{problem_id}", tags=["default_problem"]
        )
        self._setup_routes()

    def _setup_routes(self):
        @self.router.get("/", response_class=HTMLResponse)
        async def problem_statements(request: Request):
            statements = get_problem_statements(self.id, "english", "text/html")

            if statements is None:
                statements = get_problem_statements(self.id, "russian", "text/html")
                if statements is None:
                    statements = "<p>Problem statements are not available.</p>"

            problem_info = ProblemInfo(problem_id=self.id, html_statements=statements)

            return templates.TemplateResponse(
                "default_statements.html", {"request": request, "problem": problem_info}
            )

        @self.router.get("/problem-statement.css", response_class=HTMLResponse)
        async def problem_statement_css():
            css_path = os.path.join("problems", self.id, "statements", ".html")
            english_path = os.path.join(css_path, "english", "problem-statement.css")
            russian_path = os.path.join(css_path, "russian", "problem-statement.css")
            if os.path.exists(english_path):
                return FileResponse(english_path, media_type="text/css")
            elif os.path.exists(russian_path):
                return FileResponse(russian_path, media_type="text/css")
            else:
                from fastapi import HTTPException

                raise HTTPException(status_code=404, detail="CSS file not found")

        @self.router.get("/submit", response_class=HTMLResponse)
        async def problem_submit_page(request: Request):
            problem_info = ProblemInfo(
                problem_id=self.id,
            )

            return templates.TemplateResponse(
                "default_submit.html", {"request": request, "problem": problem_info}
            )

        @self.router.post("/submit", response_class=HTMLResponse)
        async def problem_submit(
            request: Request,
            language: str = Form(...),
            solution: UploadFile = Form(...),
            username: str = Depends(require_current_user),
        ):

            submission_id = str(uuid.uuid4())

            submission_dir = f"submissions/{submission_id}"
            os.makedirs(submission_dir, exist_ok=True)

            source_path = os.path.join(submission_dir, "source.cpp")
            with open(source_path, "wb") as f:
                f.write(await solution.read())

            submission_info = SubmissionRow()
            submission_info.id = submission_id
            submission_info.username = username
            submission_info.problem_id = self.id
            submission_info.language = language
            submission_info.verdict_type = "TS"
            submission_info.score = 0.0
            submission_info.send_time = datetime.now().replace(microsecond=0)

            await AsyncTableSubmissions.instance().async_add_submission(submission_info)

            log_info("app", f"Submission {submission_id} created by {username} for problem {self.id}")
            is_ok = await handle_submission(submission_id)

            if not is_ok:
                log_error("app", f"Dispatcher failed to handle submission {submission_id}")
                await AsyncTableSubmissions.instance().async_update_submission_verdict(
                    submission_id, "FAIL", 0.0
                )

            return RedirectResponse(
                url=f"/problems/{self.id}/submissions", status_code=302
            )

        @self.router.get("/submissions", response_class=HTMLResponse)
        async def problem_submissions(
            request: Request,
            username: str = Depends(require_current_user),
        ):
            submissions = await AsyncTableSubmissions.instance().async_load_submissions_by_user_and_problem(
                username, self.id
            )
            problem_info = ProblemInfo(problem_id=self.id)

            return templates.TemplateResponse(
                "default_submissions.html",
                {
                    "request": request,
                    "problem": problem_info,
                    "submissions": submissions,
                },
            )
