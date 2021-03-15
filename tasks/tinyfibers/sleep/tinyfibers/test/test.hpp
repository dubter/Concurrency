#pragma once

#include <wheels/test/test_framework.hpp>

// Run test routine in fiber scheduler

#define TINY_FIBERS_TEST(name)       \
  void FiberTestRoutine##name();     \
  SIMPLE_TEST(name) {                \
    bool exit = false;               \
    tinyfibers::RunScheduler([&]() { \
      FiberTestRoutine##name();      \
      exit = true;                   \
    });                              \
    ASSERT_TRUE(exit);               \
  }                                  \
  void FiberTestRoutine##name()
