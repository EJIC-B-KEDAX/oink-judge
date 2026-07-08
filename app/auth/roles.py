from enum import StrEnum


class Role(StrEnum):
    SUPERADMIN = "superadmin"
    ADMIN = "admin"
    USER = "user"
    BANNED = "banned"
    DELETED = "deleted"


LOGIN_BLOCKED_ROLES = frozenset({Role.BANNED, Role.DELETED})

ADMIN_ROLES = frozenset({Role.ADMIN, Role.SUPERADMIN})


def can_login(role: Role) -> bool:
    return role not in LOGIN_BLOCKED_ROLES


def can_assign_role(actor: Role, target: Role) -> bool:
    if actor == Role.SUPERADMIN:
        return True
    if actor == Role.ADMIN:
        return target in {Role.USER, Role.BANNED}
    return False
