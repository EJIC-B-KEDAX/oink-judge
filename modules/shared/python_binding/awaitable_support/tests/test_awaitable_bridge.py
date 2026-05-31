"""Tests for AwaitableBridge lifecycle and asyncio integration.

Uses the installed oink_judge.pybind11_awaitable_support module — no C++ test
helpers or pybind11::embed needed.

Run with:
    pytest test_awaitable_bridge.py -v
"""

import asyncio

import pytest
from oink_judge import pybind11_awaitable_support as m


def test_has_current_false_by_default():
    """has_current_bridge() returns False when no bridge is active."""
    assert not m.has_current_bridge()


def test_activate_outside_event_loop_raises():
    """activate() raises RuntimeError when called outside a running event loop."""
    b = m.AwaitableBridge()
    with pytest.raises(RuntimeError):
        b.activate()


def test_has_current_true_after_activate():
    """has_current_bridge() returns True immediately after activate()."""

    async def run():
        b = m.AwaitableBridge()
        b.activate()
        result = m.has_current_bridge()
        b.deactivate()
        return result

    assert asyncio.run(run())


def test_has_current_false_after_deactivate():
    """has_current_bridge() returns False after deactivate()."""

    async def run():
        b = m.AwaitableBridge()
        b.activate()
        b.deactivate()
        return m.has_current_bridge()

    assert not asyncio.run(run())


def test_double_activation_raises():
    """Calling activate() twice on the same bridge raises RuntimeError."""

    async def run():
        b = m.AwaitableBridge()
        b.activate()
        try:
            with pytest.raises(RuntimeError):
                b.activate()
        finally:
            b.deactivate()

    asyncio.run(run())


def test_deactivate_without_activate_raises():
    """Calling deactivate() without a preceding activate() raises RuntimeError."""

    async def run():
        b = m.AwaitableBridge()
        with pytest.raises(RuntimeError):
            b.deactivate()

    asyncio.run(run())


def test_bridge_reuse():
    """A bridge can be activated and deactivated multiple times in sequence."""

    async def run():
        b = m.AwaitableBridge()
        for _ in range(3):
            b.activate()
            assert m.has_current_bridge()
            b.deactivate()
            assert not m.has_current_bridge()

    asyncio.run(run())


def test_poll_noop_when_nothing_pending():
    """poll() completes without blocking or raising when there is no pending work."""

    async def run():
        b = m.AwaitableBridge()
        b.activate()
        b.poll()
        b.poll()
        b.deactivate()

    asyncio.run(run())


def test_async_with_context_manager():
    """The async-with protocol activates the bridge on enter and deactivates on exit."""

    async def run():
        async with m.AwaitableBridge():
            assert m.has_current_bridge()
        assert not m.has_current_bridge()

    asyncio.run(run())


def test_async_with_propagates_exception():
    """Exceptions raised inside async-with still cause deactivation on exit."""

    async def run():
        with pytest.raises(ValueError):
            async with m.AwaitableBridge():
                assert m.has_current_bridge()
                raise ValueError("test error")
        assert not m.has_current_bridge()

    asyncio.run(run())
