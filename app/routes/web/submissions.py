import json
from pathlib import Path

from fastapi import APIRouter, HTTPException, Request
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates

router = APIRouter(prefix="/submissions", tags=["submissions"])
templates = Jinja2Templates(directory="templates/submissions")

SUBMISSIONS_DIR = Path("submissions")


@router.get("/{submission_id}", response_class=HTMLResponse)
async def get_submission_source(request: Request, submission_id: str):
    submission_dir = SUBMISSIONS_DIR / str(submission_id)

    source_path = submission_dir / "source.cpp"
    protocol_path = submission_dir / "protocol.json"

    if not source_path.exists():
        raise HTTPException(status_code=404, detail="Submission not found")

    code = source_path.read_text(encoding="utf-8")

    protocol = None
    tests = None

    if protocol_path.exists():
        protocol = json.loads(protocol_path.read_text(encoding="utf-8"))

        tests = sorted(
            protocol["additional_info"].items(), key=lambda item: int(item[0])
        )

    return templates.TemplateResponse(
        "view_code.html",
        {
            "request": request,
            "submission_id": submission_id,
            "code": code,
            "protocol": protocol,
            "tests": tests,
        },
    )
