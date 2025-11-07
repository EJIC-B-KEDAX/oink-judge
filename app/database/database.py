from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession
from sqlalchemy.orm import sessionmaker
from app.config.config import Config, Credentials


DATABASE_URL = f"postgresql+asyncpg://{Config["database"]["username"]}:{Credentials["database"]["password"]}@{Config["database"]["host"]}/{Config["database"]["dbname"]}"

engine = create_async_engine(DATABASE_URL, echo=True, future=True)
LocalSession = sessionmaker(bind=engine, class_=AsyncSession, expire_on_commit=False)

async def get_db():
    async with LocalSession() as session:
        yield session
