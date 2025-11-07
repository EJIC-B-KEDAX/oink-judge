from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column
from sqlalchemy import String, select

class Base(DeclarativeBase):
    pass

class User(Base):
    __tablename__ = 'users'

    username: Mapped[str] = mapped_column(String, primary_key=True)
    password: Mapped[str] = mapped_column(String, nullable=False)



async def user_exists(db, username: str) -> bool:
    stmt = select(User).where(User.username == username)
    result = await db.execute(stmt)
    user = result.scalar_one_or_none()
    return user is not None



async def authenticate(db, username: str, password_hash: str) -> bool:
    stmt = select(User).where(User.username == username, User.password == password_hash)
    result = await db.execute(stmt)
    user = result.scalar_one_or_none()
    return user is not None



async def register_user(db, username: str, password_hash: str) -> bool:
    if await user_exists(db, username):
        return False

    new_user = User(username=username, password=password_hash)
    db.add(new_user)
    await db.commit()
    return True



async def delete_user(db, username: str) -> bool:
    stmt = select(User).where(User.username == username)
    result = await db.execute(stmt)
    user = result.scalar_one_or_none()

    if user is None:
        return False

    await db.delete(user)
    await db.commit()
    return True



async def update_password(db, username: str, new_password_hash: str) -> bool:
    stmt = select(User).where(User.username == username)
    result = await db.execute(stmt)
    user = result.scalar_one_or_none()

    if user is None:
        return False

    user.password = new_password_hash
    db.add(user)
    await db.commit()
    return True
