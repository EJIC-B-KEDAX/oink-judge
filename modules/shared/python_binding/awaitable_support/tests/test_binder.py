"""
Integration tests for AwaitableBinder / bindAwaitable template API.

Tests the three specialisations of AwaitableBinder:
  - Free function:          awaitable<Result>(*)(Args...)
  - Non-const member func:  awaitable<Result>(Class::*)(Args...)
  - Const member func:      awaitable<Result>(Class::*)(Args...) const

Also covers the void-return branch of awaitAndResolve, timer-based suspension,
concurrent gather, and sequential chaining.

binder_test_module is a C++ pybind11 extension built by CMake.  CTest sets
PYTHONPATH to the build directory automatically.  For manual runs:
    PYTHONPATH=<build>/modules/shared/python_binding/awaitable_support/tests \
        pytest test_binder.py -v
"""

import asyncio

import oink_judge.binder_test_module as bm
import pytest
from oink_judge.binder_test_module import Counter
from oink_judge.pybind11_awaitable_support import AwaitableBridge

# =============================================================================
# Free function — bindAwaitable(&freeDouble)
# =============================================================================


def test_free_function_int():
    async def run():
        async with AwaitableBridge():
            return await bm.async_free_double(21)

    assert asyncio.run(run()) == 42


# =============================================================================
# Test 2: Free function — void return type resolves to None
# =============================================================================


def test_free_function_void():
    async def run():
        async with AwaitableBridge():
            return await bm.async_free_void()

    assert asyncio.run(run()) is None


# =============================================================================
# Free function — string return type
# =============================================================================


def test_free_function_string():
    async def run():
        async with AwaitableBridge():
            return await bm.async_free_string("hello")

    assert asyncio.run(run()) == "hello_bound"


# =============================================================================
# Free function — timer-based suspension (Boost.Asio steady_timer)
# =============================================================================


def test_timer_based_awaitable():
    """A coroutine that suspends on a Boost.Asio timer is driven to completion."""
    import time

    async def run():
        async with AwaitableBridge():
            start = time.monotonic()
            result = await bm.async_timer(50, 99)
            elapsed_ms = (time.monotonic() - start) * 1000
            return result, elapsed_ms

    result, elapsed_ms = asyncio.run(run())
    assert result == 99
    assert elapsed_ms >= 40.0, f"Expected ≥40 ms elapsed, got {elapsed_ms:.1f} ms"


# =============================================================================
# Non-const member function — bindAwaitable(&Counter::increment)
# =============================================================================


def test_member_function_nonconst():
    async def run():
        async with AwaitableBridge():
            c = Counter(10)
            v1 = await c.increment(5)  # 10 + 5 = 15
            v2 = await c.increment(3)  # 15 + 3 = 18
            return v1, v2

    r1, r2 = asyncio.run(run())
    assert r1 == 15
    assert r2 == 18


# =============================================================================
# Const member function — bindAwaitable(&Counter::getValueConst)
# =============================================================================


def test_member_function_const():
    async def run():
        async with AwaitableBridge():
            c = Counter(7)
            return await c.get_value_const()

    assert asyncio.run(run()) == 7


# =============================================================================
# Non-const void member function — bindAwaitable(&Counter::reset)
# =============================================================================


def test_member_function_void():
    async def run():
        async with AwaitableBridge():
            c = Counter(99)
            none_val = await c.reset()
            value_after = await c.get_value_const()
            return none_val, value_after

    none_val, value_after = asyncio.run(run())
    assert none_val is None
    assert value_after == 0


# =============================================================================
# Concurrency — asyncio.gather resolves all concurrent coroutines
# =============================================================================


def test_concurrent_gather():
    async def run():
        async with AwaitableBridge():
            return await asyncio.gather(
                bm.async_free_double(1),
                bm.async_free_double(2),
                bm.async_free_double(3),
            )

    assert asyncio.run(run()) == [2, 4, 6]


# =============================================================================
# Sequential chained calls
# =============================================================================


def test_sequential_chained_calls():
    async def run():
        async with AwaitableBridge():
            a = await bm.async_free_double(5)  # 5 * 2 = 10
            b = await bm.async_timer(20, 7)  # 7 after 20 ms
            c = await bm.async_free_double(a + b)  # (10 + 7) * 2 = 34
            return c

    assert asyncio.run(run()) == 34


# =============================================================================
# Error path — call without active bridge raises RuntimeError
# =============================================================================


def test_call_outside_bridge_raises():
    async def run():
        with pytest.raises(RuntimeError):
            bm.async_free_double(1)

    asyncio.run(run())
