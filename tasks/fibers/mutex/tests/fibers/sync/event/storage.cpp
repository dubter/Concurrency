#include <twist/test/with/wheels/stress.hpp>

#include <exe/executors/thread_pool.hpp>

#include <exe/fibers/sched/go.hpp>
#include <exe/fibers/sync/event.hpp>

#include <twist/test/budget.hpp>

using namespace exe;

//////////////////////////////////////////////////////////////////////

void StorageTest() {
  executors::ThreadPool scheduler{5};
  scheduler.Start();

  while (twist::test::KeepRunning()) {
    fibers::Go(scheduler, [] {
      auto* event = new fibers::Event{};

      fibers::Go([event] {
        event->Fire();
      });

      event->Wait();
      delete event;
    });

    scheduler.WaitIdle();
  }

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Event) {
  TWIST_TEST(Event, 5s) {
    StorageTest();
  }
}

RUN_ALL_TESTS();
