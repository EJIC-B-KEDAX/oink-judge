import psycopg2
from psycopg2 import sql

# Глобальное соединение (открывается один раз при импорте)
_conn = None

def init_db(
    dbname="oink_judge_db",
    user="oink_judge",
    password="OOIINNKK4242",
    host="192.168.0.142",
    port=5432
):
    """
    Инициализация подключения к базе и таблицы submissions.
    Вызывать один раз при старте программы.
    """
    global _conn
    if _conn is None or _conn.closed:
        _conn = psycopg2.connect(
            dbname=dbname,
            user=user,
            password=password,
            host=host,
            port=port
        )
        _conn.autocommit = True  # чтобы CREATE TABLE применялся сразу

        with _conn.cursor() as cur:
            cur.execute("""
                CREATE TABLE IF NOT EXISTS submissions (
                    submission_id TEXT PRIMARY KEY,
                    username TEXT,
                    problem_id TEXT,
                    language TEXT,
                    verdict_type TEXT,
                    score REAL
                )
            """)

def insert_submission(submission_id, username, problem_id, language, verdict_type, score):
    """
    Вставка строки в таблицу submissions с использованием глобального соединения.
    """
    global _conn
    if _conn is None or _conn.closed:
        raise RuntimeError("База данных не инициализирована. Сначала вызови init_db().")

    with _conn.cursor() as cur:
        query = sql.SQL("""
            INSERT INTO submissions (submission_id, username, problem_id, language, verdict_type, score)
            VALUES (%s, %s, %s, %s, %s, %s)
        """)
        cur.execute(query, (submission_id, username, problem_id, language, verdict_type, score))
    _conn.commit()

def get_submission_result(submission_id):
    """
    Возвращает (verdict_type, score) по submission_id.
    Если записи нет, возвращает None.
    """
    global _conn
    if _conn is None or _conn.closed:
        raise RuntimeError("База данных не инициализирована. Сначала вызови init_db().")

    with _conn.cursor() as cur:
        query = sql.SQL("""
            SELECT verdict_type, score
            FROM submissions
            WHERE submission_id = %s
        """)
        cur.execute(query, (submission_id,))
        row = cur.fetchone()
        return row if row else None  # (verdict_type, score) или None

def close_db():
    """
    Закрывает глобальное соединение. Вызывать при завершении программы.
    """
    global _conn
    if _conn and not _conn.closed:
        _conn.close()
        _conn = None