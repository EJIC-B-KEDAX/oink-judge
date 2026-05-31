"""Tests for the pybind11 logger module — Python logging integration."""

import io
import logging

import pytest
from oink_judge.pybind11_logger import (
    Logger,
    LogType,
    disable_colors,
    enable_colors,
    log_error,
    log_info,
    log_message,
    log_success,
)


@pytest.fixture(autouse=True)
def _reset_logger():
    """Reset Logger singleton state before each test."""
    logger = Logger.instance()
    logger.set_log_level("default", 0)
    enable_colors()
    yield


def _make_handler(logger_name: str) -> tuple[logging.StreamHandler, logging.Logger]:
    """Create a StreamHandler attached to a named logger and return both."""
    stream = io.StringIO()
    handler = logging.StreamHandler(stream)
    handler.setLevel(logging.DEBUG)
    handler.setFormatter(logging.Formatter("%(levelname)s:%(message)s"))
    py_logger = logging.getLogger(logger_name)
    py_logger.handlers.clear()
    py_logger.addHandler(handler)
    py_logger.setLevel(logging.DEBUG)
    py_logger.propagate = False
    return handler, py_logger


# ---------------------------------------------------------------------------
# log_message routes through Python logging
# ---------------------------------------------------------------------------
class TestLogMessageRouting:
    def test_message_reaches_python_handler(self):
        Logger.instance().set_log_level("route_test", 1)
        handler, _ = _make_handler("route_test")
        log_message("route_test", "hello from C++", LogType.INFO, 1)
        output = handler.stream.getvalue()
        assert "hello from C++" in output

    # Case 3: level filtering
    def test_level_filtering(self):
        Logger.instance().set_log_level("filter_test", 1)
        handler, _ = _make_handler("filter_test")
        log_message("filter_test", "visible", LogType.INFO, 1)
        log_message("filter_test", "hidden", LogType.INFO, 2)
        output = handler.stream.getvalue()
        assert "visible" in output
        assert "hidden" not in output


# ---------------------------------------------------------------------------
# LogType to Python level mapping
# ---------------------------------------------------------------------------
class TestLogTypeToPythonLevel:
    @pytest.mark.parametrize(
        ("log_type", "expected_level"),
        [
            (LogType.DEBUG, "DEBUG"),
            (LogType.INFO, "INFO"),
            (LogType.SUCCESS, "INFO"),  # SUCCESS maps to Python INFO
            (LogType.WARNING, "WARNING"),
            (LogType.ERROR, "ERROR"),
            (LogType.CRITICAL, "CRITICAL"),
        ],
        ids=["debug", "info", "success", "warning", "error", "critical"],
    )
    def test_log_type_maps_to_python_level(self, log_type, expected_level):
        mod_name = f"level_map_{log_type.name}"
        Logger.instance().set_log_level(mod_name, 1)
        handler, _ = _make_handler(mod_name)
        log_message(mod_name, "test_msg", log_type, 1)
        output = handler.stream.getvalue()
        assert output.startswith(expected_level + ":"), (
            f"Expected {expected_level}, got: {output!r}"
        )


# ---------------------------------------------------------------------------
# Python convenience functions
# ---------------------------------------------------------------------------
class TestPythonConvenienceFunctions:
    def test_log_error_routes_at_error_level(self):
        Logger.instance().set_log_level("conv_err", 1)
        handler, _ = _make_handler("conv_err")
        log_error("conv_err", "fail")
        output = handler.stream.getvalue()
        assert "ERROR:fail" in output

    def test_log_info_routes_at_info_level(self):
        Logger.instance().set_log_level("conv_info", 1)
        handler, _ = _make_handler("conv_info")
        log_info("conv_info", "note")
        output = handler.stream.getvalue()
        assert "INFO:note" in output

    def test_log_success_routes_at_info_level(self):
        Logger.instance().set_log_level("conv_succ", 1)
        handler, _ = _make_handler("conv_succ")
        log_success("conv_succ", "ok")
        output = handler.stream.getvalue()
        assert "INFO:ok" in output


# ---------------------------------------------------------------------------
# Configuration from Python
# ---------------------------------------------------------------------------
class TestPythonConfiguration:
    def test_set_get_log_level_round_trip(self):
        Logger.instance().set_log_level("roundtrip_mod", 42)
        assert Logger.instance().get_log_level("roundtrip_mod") == 42

    def test_disable_colors_clears_color_map(self):
        enable_colors()
        disable_colors()
        color_map = Logger.instance().get_color_map()
        for value in color_map.values():
            assert value == "", f"Expected empty color, got {value!r}"

    def test_enable_colors_restores_defaults(self):
        disable_colors()
        enable_colors()
        color_map = Logger.instance().get_color_map()
        assert "\033[" in color_map.get("DEBUG", ""), (
            "Expected ANSI codes after enable_colors"
        )

    def test_default_constants_accessible(self):
        assert Logger.DEFAULT_MIN_LOCATION_LENGTH == 40
        assert Logger.DEFAULT_MIN_MODULE_LENGTH == 20
