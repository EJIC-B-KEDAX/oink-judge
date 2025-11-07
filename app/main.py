from fastapi import FastAPI
from app.routes.web import auth, main
from app.routes.web.problems import default_problem


app = FastAPI()

app.include_router(auth.router)
app.include_router(main.router)
app.include_router(default_problem.router)
