#include <tp/thread_pool.hpp>

#include <twist/test/test.hpp>
#include <twist/test/runs.hpp>
#include <twist/test/util/race.hpp>

#include <wheels/test/util.hpp>

#include <twist/stdlike/atomic.hpp>

#include <atomic>

////////////////////////////////////////////////////////////////////////////////

namespace tasks {

void KeepAlive() {
  if (twist::test::KeepRunning()) {
    tp::Current()->Submit([]() {
      KeepAlive();
    });
  }
}

void Backoff() {
  twist::strand::stdlike::this_thread::yield();
}

void Test(size_t threads, size_t clients, size_t limit) {
  tp::ThreadPool pool{threads};

  pool.Submit([]() {
    KeepAlive();
  });

  std::atomic<size_t> completed{0};

  twist::stdlike::atomic<size_t> queue{0};

  twist::test::util::Race race;

  for (size_t i = 0; i < clients; ++i) {
    race.Add([&]() {
      while (twist::test::KeepRunning()) {
        // TrySubmit
        if (++queue <= limit) {
          pool.Submit([&]() {
            --queue;
            ++completed;
          });
        } else {
          --queue;
          Backoff();
        }
      }
    });
  }

  race.Run();

  pool.WaitIdle();
  pool.Stop();

  std::cout << "Tasks completed: " << completed.load() << std::endl;

  ASSERT_EQ(queue.load(), 0);
  ASSERT_GT(completed.load(), 8888);
}

}  // namespace tasks

TWIST_TEST_RUNS(Submits, tasks::Test)
  ->TimeLimit(4s)
  ->Run(3, 5, 111)
  ->Run(4, 3, 13)
  ->Run(2, 4, 5)
  ->Run(9, 10, 33);

////////////////////////////////////////////////////////////////////////////////

namespace wait_idle {

void TestOneTask() {
  tp::ThreadPool pool{4};

  while (wheels::test::KeepRunning()) {
    size_t tasks = 0;

    pool.Submit([&]() {
      ++tasks;
    });

    pool.WaitIdle();

    ASSERT_EQ(tasks, 1);
  }

  pool.Stop();
}

void TestSeries() {
  tp::ThreadPool pool{1};

  size_t iter = 0;

  while (wheels::test::KeepRunning()) {
    ++iter;
    const size_t tasks = 1 + iter % 3;

    size_t tasks_completed = 0;
    for (size_t i = 0; i < tasks; ++i) {
      pool.Submit([&](){
        ++tasks_completed;
      });
    }

    pool.WaitIdle();

    ASSERT_EQ(tasks_completed, tasks);
  }

  pool.Stop();
}

void TestCurrent() {
  tp::ThreadPool pool{2};

  while (wheels::test::KeepRunning()) {
    bool done = false;

    pool.Submit([&]() {
      tp::ThreadPool::Current()->Submit([&]() {
        done = true;
      });
    });

    pool.WaitIdle();

    ASSERT_TRUE(done);
  }

  pool.Stop();
}

void TestConcurrent() {
  tp::ThreadPool pool{2};

  size_t tasks = 0;

  twist::stdlike::thread t1([&]() {
    pool.Submit([&]() {
      ++tasks;
    });
  });

  twist::stdlike::thread t2([&]() {
    pool.WaitIdle();
  });

  t1.join();
  t2.join();

  ASSERT_TRUE(tasks <= 1);

  pool.Stop();
}

}  // namespace wait_idle

TEST_SUITE(WaitIdle) {
  TWIST_TEST_TL(OneTask, 5s) {
    wait_idle::TestOneTask();
  }

  TWIST_TEST_TL(Series, 5s) {
    wait_idle::TestSeries();
  }

  TWIST_TEST_TL(Current, 5s) {
    wait_idle::TestCurrent();
  }

  TWIST_ITERATE_TEST(Concurrent, 5s) {
    wait_idle::TestConcurrent();
  }
}
