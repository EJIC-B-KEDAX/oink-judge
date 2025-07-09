import json
from config import config

class SubmissionInfo:
    def __init__(self, participant: str, language: str, problem_id: str, sending_time: int):
        self.participant = participant
        self.language = language
        self.problem_id = problem_id
        self.sending_time = sending_time


def load_submission_info(submission_id: str) -> SubmissionInfo:
    info = json.load(open(config.get_directory("submissions") + "/" + submission_id + "/" + "info.json", 'r'))

    return SubmissionInfo(info["participant"], info["language"], info["problem_id"], info["sending_time"])

def store_submission_info(submission_id: str, submission_info: SubmissionInfo):
    info = {"participant": submission_info.participant,
            "language": submission_info.language,
            "problem_id": submission_info.problem_id,
            "sending_time": submission_info.sending_time}

    json.dump(info, open(config.get_directory("submissions") + "/" + submission_id + "/" + "info.json", 'w'))