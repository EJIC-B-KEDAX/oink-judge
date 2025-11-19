from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column
from sqlalchemy import String, Float, select, DateTime
from datetime import datetime

class Base(DeclarativeBase):
    pass

class Submission(Base):
    __tablename__ = 'submissions'

    id: Mapped[str] = mapped_column(String, primary_key=True)
    username: Mapped[str] = mapped_column(String, nullable=False)
    problem_id: Mapped[str] = mapped_column(String, nullable=False)
    language: Mapped[str] = mapped_column(String, nullable=False)
    verdict_type: Mapped[str] = mapped_column(String, nullable=False)
    score: Mapped[float] = mapped_column(Float, nullable=False)
    send_time: Mapped[datetime] = mapped_column(DateTime, nullable=False)

class SubmissionInfo:
    def __init__(self, id: str, username: str, problem_id: str, language: str, verdict_type: str, score: float, send_time: float):
        self.id = id
        self.username = username
        self.problem_id = problem_id
        self.language = language
        self.verdict_type = verdict_type
        self.score = score
        self.send_time = send_time



async def add_submission(db, submission_info: SubmissionInfo) -> bool:
    new_submission = Submission(
        id=submission_info.id,
        username=submission_info.username,
        problem_id=submission_info.problem_id,
        language=submission_info.language,
        verdict_type=submission_info.verdict_type,
        score=submission_info.score,
        send_time=submission_info.send_time
    )
    db.add(new_submission)
    await db.commit()
    return True



async def load_submission_by_id(db, submission_id: str) -> SubmissionInfo | None:
    stmt = select(Submission).where(Submission.id == submission_id)
    result = await db.execute(stmt)
    submission = result.scalar_one_or_none()

    if submission is None:
        return None

    return SubmissionInfo(
        id=submission.id,
        username=submission.username,
        problem_id=submission.problem_id,
        language=submission.language,
        verdict_type=submission.verdict_type,
        score=submission.score,
        send_time=submission.send_time
    )


async def update_submission_verdict(db, submission_id: str, verdict_type: str, score: float) -> bool:
    stmt = select(Submission).where(Submission.id == submission_id)
    result = await db.execute(stmt)
    submission = result.scalar_one_or_none()

    if submission is None:
        return False

    submission.verdict_type = verdict_type
    submission.score = score

    db.add(submission)
    await db.commit()
    return True



async def load_submissions_by_user_and_problem(db, username: str, problem_id: str) -> list[SubmissionInfo]:
    stmt = select(Submission).where(
        (Submission.username == username) &
        (Submission.problem_id == problem_id)
    ).order_by(Submission.send_time.desc())
    
    result = await db.execute(stmt)
    submissions = result.scalars().all()

    submission_infos = []
    for submission in submissions:
        submission_infos.append(SubmissionInfo(
            id=submission.id,
            username=submission.username,
            problem_id=submission.problem_id,
            language=submission.language,
            verdict_type=submission.verdict_type,
            score=submission.score,
            send_time=submission.send_time
        ))

    return submission_infos