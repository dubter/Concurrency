#include <exe/executors/manual.hpp>
#include <exe/executors/thread_pool.hpp>

#include <exe/tasks/core/task.hpp>
#include <exe/tasks/run/fire.hpp>
#include <exe/tasks/run/teleport.hpp>
#include <exe/tasks/run/yield.hpp>

#include <exe/tasks/sync/wait_group.hpp>

#include <wheels/test/framework.hpp>

#include <wheels/support/cpu_time.hpp>

#include <iostream>
#include <thread>

using namespace exe;

TEST_SUITE(TaskWaitGroup) {
  SIMPLE_TEST(OneWaiter) {
    executors::ManualExecutor scheduler;

    tasks::WaitGroup wg;

    bool waiter_done = false;
    bool worker_done = false;

    auto waiter = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      co_await wg.Wait();

      ASSERT_TRUE(worker_done);
      waiter_done = true;
    };

    tasks::FireAndForget(waiter());

    auto worker = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      for (size_t i = 0; i < 10; ++i) {
        co_await tasks::Yield(scheduler);
      }

      worker_done = true;

      wg.Done();
    };

    wg.Add(1);
    tasks::FireAndForget(worker());

    scheduler.Drain();

    ASSERT_TRUE(waiter_done);
    ASSERT_TRUE(worker_done);
  }

  SIMPLE_TEST(Workers) {
    executors::ManualExecutor scheduler;

    tasks::WaitGroup wg;

    static const size_t kWorkers = 3;
    static const size_t kWaiters = 4;
    static const size_t kYields = 10;

    size_t waiters_done = 0;
    size_t workers_done = 0;

    auto waiter = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      co_await wg.Wait();

      ASSERT_EQ(workers_done, kWorkers);
      ++waiters_done;
    };

    for (size_t i = 0; i < kWaiters; ++i) {
      tasks::FireAndForget(waiter());
    }

    auto worker = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      for (size_t i = 0; i < kYields; ++i) {
        co_await tasks::Yield(scheduler);
      }

      ++workers_done;

      wg.Done();
    };

    wg.Add(kWorkers);
    for (size_t j = 0; j < kWorkers; ++j) {
      tasks::FireAndForget(worker());
    }

    size_t steps = scheduler.Drain();

    ASSERT_EQ(workers_done, kWorkers);
    ASSERT_EQ(waiters_done, kWaiters);

    ASSERT_GE(steps, kWaiters + kWorkers * kYields);
  }

  SIMPLE_TEST(BlockingWait) {
    executors::ThreadPool scheduler{/*threads=*/4};

    tasks::WaitGroup wg;

    std::atomic<size_t> workers = 0;

    static const size_t kWorkers = 3;

    wg.Add(kWorkers);

    auto waiter = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      co_await wg.Wait();
      ASSERT_EQ(workers.load(), kWorkers);
      co_return;
    };

    auto worker = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      std::this_thread::sleep_for(1s);
      ++workers;
      wg.Done();
    };

    wheels::ProcessCPUTimer timer;

    tasks::FireAndForget(waiter());
    for (size_t i = 0; i < kWorkers; ++i) {
      tasks::FireAndForget(worker());
    }

    scheduler.WaitIdle();

    ASSERT_TRUE(timer.Elapsed() < 100ms);

    scheduler.Stop();
  }
}

RUN_ALL_TESTS()
