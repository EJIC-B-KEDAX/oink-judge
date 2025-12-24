from fastapi import FastAPI
from app.routes.web import auth, main
from app.routes.web.problems import open_problem
from app.config.config import Config
import os


app = FastAPI()

app.include_router(auth.router)
app.include_router(main.router)

for problem_id in os.listdir(Config["directories"]["problems"]):
    problem = open_problem.OpenProblem(problem_id) # TODO handle different problem types (make factory function)
    app.include_router(problem.router)