import os
from contextlib import asynccontextmanager

from fastapi import FastAPI
from oink_judge.pybind11_logger import log_info
from oink_judge.pybind11_plugin_manager import PluginManager, get_all_plugin_paths

from app.awaitable_bridge import awaitable_bridge_lifespan
from app.config import (
    configure_logger,
    get_directory_path,
    get_logger_config,
    set_config_file_path,
    set_credentials_file_path,
)
from app.auth.api.router import router as auth_api_router
from app.routes.web import auth, main, submissions
from app.routes.web.problems import open_problem

plugin_manager = PluginManager()
for plugin_path in get_all_plugin_paths():
    plugin_manager.load(plugin_path)


@asynccontextmanager
async def lifespan(_app: FastAPI):
    async with awaitable_bridge_lifespan():
        yield


app = FastAPI(lifespan=lifespan)

app.include_router(auth_api_router)
app.include_router(submissions.router)
app.include_router(auth.router)
app.include_router(main.router)

set_config_file_path("configs/app/config.json")
set_credentials_file_path("configs/app/credentials.json")

logger_config = get_logger_config()
if logger_config is not None:
    configure_logger(logger_config)

log_info("app", "Starting web application")

for problem_id in os.listdir(get_directory_path("problems")):
    problem = open_problem.OpenProblem(
        problem_id
    )  # TODO handle different problem types (make factory function)
    app.include_router(problem.router)
