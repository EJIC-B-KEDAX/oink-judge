import xml.etree.ElementTree as ET
from app.config.config import Config

cached_problem_configs = {}

def load_problem_config(problem_id: str):
    global cached_problem_configs

    if problem_id in cached_problem_configs:
        return cached_problem_configs[problem_id]

    problems_dir: str = Config["directories"]["problems"]
    problem_config_path = f"{problems_dir}/{problem_id}/problem.xml"

    tree = ET.parse(problem_config_path)
    root = tree.getroot()

    cached_problem_configs[problem_id] = root
    return root



def get_path_to_problem_statements(problem_id: str, language: str, type: str) -> str | None:
    problem_config = load_problem_config(problem_id)
    problems_dir: str = Config["directories"]["problems"]

    statements = problem_config.find("statements")
    for statement in statements.findall("statement"):
        if statement.get("language") == language and statement.get("type") == type:
            return f"{problems_dir}/{problem_id}/{statement.get("path")}"
        
    return None



def get_problem_statements(problem_id: str, language: str, type: str) -> str | None:
    path_to_statement = get_path_to_problem_statements(problem_id, language, type)
    if path_to_statement is None:
        return None
    else:
        with open(path_to_statement, "r", encoding="utf-8") as f:
            return f.read()