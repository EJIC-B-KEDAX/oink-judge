from sqlalchemy.ext.asyncio import AsyncSession, create_async_engine
from sqlalchemy.orm import sessionmaker

DATABASE_URL = f"postgresql+asyncpg://{Config['database']['username']}:{Credentials['database']['password']}@{Config['database']['host']}/{Config['database']['dbname']}"

engine = create_async_engine(DATABASE_URL, future=True)
LocalSession = sessionmaker(bind=engine, class_=AsyncSession, expire_on_commit=False)


async def get_db():
    async with LocalSession() as session:
        yield session
