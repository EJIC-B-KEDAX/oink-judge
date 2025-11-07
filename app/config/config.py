import json

CONFIG_FILE_PATH = "configs/app/config.json"
CREDENTIALS_FILE_PATH = "configs/app/credentials.json"

Config = json.load(open(CONFIG_FILE_PATH, "r"))
Credentials = json.load(open(CREDENTIALS_FILE_PATH, "r"))
