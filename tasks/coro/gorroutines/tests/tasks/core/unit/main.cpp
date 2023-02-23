#include <exe/executors/manual.hpp>
#include <exe/executors/thread_pool.hpp>

#include <exe/tasks/core/task.hpp>
#include <exe/tasks/run/fire.hpp>
#include <exe/tasks/run/teleport.hpp>
#include <exe/tasks/run/yield.hpp>

#include <wheels/test/framework.hpp>

#include <iostream>

using namespace exe;

TEST_SUITE(GorRoutines) {
  SIMPLE_TEST(Fire) {
    auto done = false;

    auto gorroutine = [&]() -> tasks::Task<> {
      std::cout << "Hi" << std::endl;
      done = true;
      co_return;
    };

    auto task = gorroutine();

    ASSERT_FALSE(done);

    tasks::FireAndForget(std::move(task));

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(Teleport) {
    executors::ManualExecutor scheduler;

    bool done = false;

    auto gorroutine = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      done = true;
    };

    tasks::FireAndForget(gorroutine());

    ASSERT_FALSE(done);

    ASSERT_TRUE(scheduler.RunNext());

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(Yield) {
    executors::ManualExecutor scheduler;

    bool done = false;

    auto gorroutine = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      for (size_t i = 0; i < 10; ++i) {
        co_await tasks::Yield(scheduler);
      }

      done = true;
    };

    auto task = gorroutine();

    tasks::FireAndForget(std::move(task));

    size_t steps = scheduler.Drain();
    ASSERT_EQ(steps, 1 + 10);

    ASSERT_TRUE(done);
  }
}

RUN_ALL_TESTS()
