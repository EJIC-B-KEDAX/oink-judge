from collections.abc import AsyncIterator
from contextlib import AbstractAsyncContextManager, asynccontextmanager

from oink_judge.pybind11_database import ConnectionPool

from app.services.auth.table_sessions import TableSessions
from app.services.auth.table_users import TableUsers

_initialized = False


@asynccontextmanager
async def _database_session() -> AsyncIterator[None]:
    global _initialized

    if not _initialized:
        await ConnectionPool.instance().async_initialize()
        await TableUsers.instance().initialize()
        await TableSessions.instance().initialize()
        _initialized = True
    yield


def database_session() -> AbstractAsyncContextManager[None]:
    return _database_session()
