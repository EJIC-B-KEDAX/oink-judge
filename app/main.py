import os

from fastapi import FastAPI

from app.config import (
    get_directory_path,
    set_config_file_path,
    set_credentials_file_path,
)
from app.routes.web import auth, main, submissions
from app.routes.web.problems import open_problem

app = FastAPI()

app.include_router(submissions.router)
app.include_router(auth.router)
app.include_router(main.router)

set_config_file_path("configs/app/config.json")
set_credentials_file_path("configs/app/credentials.json")

for problem_id in os.listdir(get_directory_path("problems")):
    problem = open_problem.OpenProblem(
        problem_id
    )  # TODO handle different problem types (make factory function)
    app.include_router(problem.router)
