import json

CONFIG_FILE_PATH = "../config.json"

class Config:
    def __init__(self):
        self._config_data = json.load(open(CONFIG_FILE_PATH, 'r'))

    def get_port(self, key: str):
        return self._config_data["ports"][key]

    def get_directory(self, key: str):
        return self._config_data["directories"][key]

    def get_bound(self, key: str):
        return self._config_data["bounds"][key]

config = Config()