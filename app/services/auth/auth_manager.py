from app.services.auth.session import Session
from app.services.auth.table_sessions import TableSessions
from app.services.auth.table_users import TableUsers


class AuthManager:
    _instance: "AuthManager | None" = None

    @classmethod
    def instance(cls) -> "AuthManager":
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    async def register_user(self, username: str, password: str) -> bool:
        return await TableUsers.instance().register_user(username, password)

    async def user_exists(self, username: str) -> bool:
        return await TableUsers.instance().user_exists(username)

    async def delete_user(self, username: str) -> bool:
        return await TableUsers.instance().delete_user(username)

    async def update_password(self, username: str, new_password: str) -> bool:
        return await TableUsers.instance().update_password(username, new_password)

    async def authenticate(self, username: str, password: str) -> str:
        if await TableUsers.instance().authenticate(username, password):
            session = Session(username)
            session.generate_session()
            await TableSessions.instance().add_session(session)
            return session.session_id
        return ""

    async def whose_session(self, session_id: str) -> str:
        return await TableSessions.instance().whose_session(session_id)

    async def is_session_valid(self, session_id: str) -> bool:
        return bool(await self.whose_session(session_id))

    async def invalidate_session(self, session_id: str) -> None:
        await TableSessions.instance().remove_session(session_id)
