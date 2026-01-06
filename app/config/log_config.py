LOGGING_CONFIG = {
    "version": 1,
    "disable_existing_loggers": False,

    "formatters": {
        "default": {
            "format": "%(asctime)s | %(levelname)s | %(name)s | %(message)s",
            "datefmt": "%Y-%m-%d %H:%M:%S",
        },
        "access": {
            "format": "%(asctime)s | %(levelname)s | %(message)s",
            "datefmt": "%Y-%m-%d %H:%M:%S",
        },
    },

    "handlers": {
        "default": {
            "class": "logging.StreamHandler",
            "formatter": "default",
        },
        "access": {
            "class": "logging.StreamHandler",
            "formatter": "access",
        },
    },

    "root": {
        "handlers": ["default"],
        "level": "INFO",
    },

    "loggers": {
        "uvicorn": {
            "handlers": ["default"],
            "level": "INFO",
            "propagate": False,
        },
        "uvicorn.error": {
            "handlers": ["default"],
            "level": "INFO",
            "propagate": False,
        },
        "uvicorn.access": {
            "handlers": ["access"],
            "level": "INFO",
            "propagate": False,
        },
        "sqlalchemy.engine.Engine": {
            "handlers": ["default"],
            "level": "INFO",
            "propagate": False,
        },
    },
}
