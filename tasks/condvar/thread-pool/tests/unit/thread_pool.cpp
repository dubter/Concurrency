#include <tp/static_thread_pool.hpp>
#include <tp/thread_label.hpp>

#include <wheels/test/test_framework.hpp>

#include <wheels/support/cpu_time.hpp>
#include <wheels/support/time.hpp>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

TEST_SUITE(ThreadPool) {
  SIMPLE_TEST(JustWorks) {
    tp::StaticThreadPool pool{4, "test"};

    pool.Submit([]() {
      std::cout << "Hello from thread pool!" << std::endl;
    });

    pool.Join();
  }

  SIMPLE_TEST(Join) {
    tp::StaticThreadPool pool{4, "test"};

    bool done = false;

    pool.Submit([&]() {
      std::this_thread::sleep_for(1s);
      done = true;
    });

    pool.Join();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(Name) {
    tp::StaticThreadPool pool{4, "test"};

    pool.Submit([]() {
      tp::ExpectThread("test");
    });

    pool.Join();
  }

  SIMPLE_TEST(Exceptions) {
    tp::StaticThreadPool pool{1, "test"};

    pool.Submit([]() {
      throw std::runtime_error("Task failed");
    });

    pool.Join();
  }

  SIMPLE_TEST(ManyTasks) {
    tp::StaticThreadPool pool{4, "test"};

    static const size_t kTasks = 17;

    std::atomic<size_t> tasks{0};

    for (size_t i = 0; i < kTasks; ++i) {
      pool.Submit([&]() {
        ++tasks;
      });
    }

    pool.Join();

    ASSERT_EQ(tasks.load(), kTasks);
  }

  SIMPLE_TEST(Parallel) {
    tp::StaticThreadPool pool{4, "test"};

    std::atomic<size_t> tasks{0};

    pool.Submit([&]() {
      tp::ExpectThread("test");
      std::this_thread::sleep_for(1s);
      ++tasks;
    });

    pool.Submit([&]() {
      ++tasks;
    });

    std::this_thread::sleep_for(100ms);

    ASSERT_EQ(tasks.load(), 1);

    pool.Join();

    ASSERT_EQ(tasks.load(), 2);
  }

  SIMPLE_TEST(TwoPools) {
    tp::StaticThreadPool pool1{1, "first"};
    tp::StaticThreadPool pool2{1, "second"};

    std::atomic<size_t> tasks{0};

    pool1.Submit([&]() {
      tp::ExpectThread("first");
      std::this_thread::sleep_for(1s);
      ++tasks;
    });

    pool2.Submit([&]() {
      tp::ExpectThread("second");
      std::this_thread::sleep_for(1s);
      ++tasks;
    });

    pool2.Join();
    pool1.Join();

    ASSERT_EQ(tasks.load(), 2);
  }

  SIMPLE_TEST(Shutdown) {
    tp::StaticThreadPool pool{3, "test"};

    for (size_t i = 0; i < 3; ++i) {
      pool.Submit([]() {
        std::this_thread::sleep_for(1s);
      });
    }

    for (size_t i = 0; i < 10; ++i) {
      pool.Submit([]() {
        std::this_thread::sleep_for(100s);
      });
    }

    std::this_thread::sleep_for(250ms);

    pool.Shutdown();
  }

  SIMPLE_TEST(DoNotBurnCPU) {
    tp::StaticThreadPool pool{4, "test"};

    // Warmup
    for (size_t i = 0; i < 4; ++i) {
      pool.Submit([&]() {
        std::this_thread::sleep_for(100ms);
      });
    }

    wheels::ProcessCPUTimer cpu_timer;

    std::this_thread::sleep_for(1s);

    pool.Join();

    ASSERT_TRUE(cpu_timer.Elapsed() < 100ms);
  }

  SIMPLE_TEST(Current) {
    tp::StaticThreadPool pool{1, "test"};

    ASSERT_EQ(tp::Current(), nullptr);

    pool.Submit([&]() {
      ASSERT_EQ(tp::Current(), &pool);
    });

    pool.Join();
  }

  SIMPLE_TEST(SubmitFromPool) {
    tp::StaticThreadPool pool{4, "test"};

    bool done = false;

    pool.Submit([&]() {
      std::this_thread::sleep_for(500ms);
      tp::Current()->Submit([&]() {
        std::this_thread::sleep_for(500ms);
        done = true;
      });
    });

    pool.Join();

    ASSERT_TRUE(done);
  }

  TEST(UseThreads, wheels::test::TestOptions().TimeLimit(1s)) {
    tp::StaticThreadPool pool{4, "test"};

    std::atomic<size_t> tasks{0};

    for (size_t i = 0; i < 4; ++i) {
      pool.Submit([&]() {
        std::this_thread::sleep_for(750ms);
        ++tasks;
      });
    }

    pool.Join();

    ASSERT_EQ(tasks.load(), 4);
  }

  TEST(TooManyThreads, wheels::test::TestOptions().TimeLimit(2s)) {
    tp::StaticThreadPool pool{3, "test"};

    std::atomic<size_t> tasks{0};

    for (size_t i = 0; i < 4; ++i) {
      pool.Submit([&]() {
        std::this_thread::sleep_for(750ms);
        ++tasks;
      });
    }

    wheels::StopWatch stop_watch;

    pool.Join();

    ASSERT_TRUE(stop_watch.Elapsed() > 1s);
    ASSERT_EQ(tasks.load(), 4);
  }

  void KeepAlive() {
    if (wheels::test::TestTimeLeft() > 300ms) {
      tp::Current()->Submit([]() {
        KeepAlive();
      });
    }
  }

  TEST(KeepAlive, wheels::test::TestOptions().TimeLimit(4s)) {
    tp::StaticThreadPool pool{3, "test"};

    for (size_t i = 0; i < 5; ++i) {
      pool.Submit([]() {
        KeepAlive();
      });
    }

    wheels::StopWatch stop_watch;

    pool.Join();

    ASSERT_TRUE(stop_watch.Elapsed() > 3s);
  }

  SIMPLE_TEST(Racy) {
    tp::StaticThreadPool pool{4, "test"};

    std::atomic<int> shared_counter{0};
    std::atomic<int> tasks{0};

    for (size_t i = 0; i < 100500; ++i) {
      pool.Submit([&]() {
        int old = shared_counter.load();
        shared_counter.store(old + 1);

        ++tasks;
      });
    }

    pool.Join();

    ASSERT_EQ(tasks.load(), 100500);
    ASSERT_LE(shared_counter.load(), 100500);
  }
}
