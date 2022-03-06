#include <exe/tp/thread_pool.hpp>
#include <exe/tp/submit.hpp>

#include <wheels/test/test_framework.hpp>

#include <twist/test/test.hpp>
#include <twist/test/runs.hpp>
#include <twist/test/util/race.hpp>

#include <twist/stdlike/atomic.hpp>

#include <atomic>

////////////////////////////////////////////////////////////////////////////////

using exe::tp::ThreadPool;
using exe::tp::Submit;

////////////////////////////////////////////////////////////////////////////////

namespace tasks {

void KeepAlive() {
  if (twist::test::KeepRunning()) {
    Submit(*ThreadPool::Current(), []() {
      KeepAlive();
    });
  }
}

void Backoff() {
  twist::strand::stdlike::this_thread::yield();
}

void Test(size_t threads, size_t clients, size_t limit) {
  ThreadPool pool{threads};

  Submit(pool, []() {
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
          Submit(pool, [&]() {
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

namespace join {

void TestSequential() {
  ThreadPool pool{4};

  std::atomic<size_t> tasks{0};

  Submit(pool, [&]() {
    ++tasks;
  });

  pool.WaitIdle();
  pool.Stop();

  ASSERT_EQ(tasks.load(), 1);
}

void TestConcurrent() {
  ThreadPool pool{2};

  std::atomic<size_t> tasks{0};

  twist::test::util::Race race;

  race.Add([&]() {
    Submit(pool, [&]() {
      ++tasks;
    });
  });

  race.Add([&]() {
    pool.WaitIdle();
    pool.Stop();
  });

  race.Run();

  ASSERT_LE(tasks.load(), 1);
}

void TestCurrent() {
  ThreadPool pool{2};

  std::atomic<bool> done{false};

  Submit(pool, [&]() {
    Submit(*ThreadPool::Current(), [&]() {
      done = true;
    });
  });

  pool.WaitIdle();
  pool.Stop();

  ASSERT_TRUE(done.load());
}

}  // namespace join

TEST_SUITE(Join) {
  TWIST_ITERATE_TEST(Sequential, 5s) {
    join::TestSequential();
  }

  TWIST_ITERATE_TEST(Concurrent, 5s) {
    join::TestConcurrent();
  }

  TWIST_ITERATE_TEST(Current, 5s) {
    join::TestCurrent();
  }
}

RUN_ALL_TESTS()
