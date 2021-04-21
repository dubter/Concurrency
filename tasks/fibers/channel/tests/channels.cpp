#include <mtf/fibers/core/api.hpp>
#include <mtf/fibers/sync/channel.hpp>
#include <mtf/fibers/sync/select.hpp>
#include <mtf/fibers/test/test.hpp>

#include <wheels/test/test_framework.hpp>

#include <chrono>

using mtf::fibers::Channel;
using mtf::fibers::Select;
using mtf::fibers::Spawn;
using mtf::fibers::Yield;
using mtf::tp::StaticThreadPool;

using namespace std::chrono_literals;

static const auto kLongTestOptions = wheels::test::TestOptions().TimeLimit(30s);

TEST_SUITE(Channels) {
  SIMPLE_FIBER_TEST(JustWorks, 1) {
    Channel<int> ints;
    ints.Send(1);
    ints.Send(2);
    ints.Send(3);
    ASSERT_EQ(ints.Receive(), 1);
    ASSERT_EQ(ints.Receive(), 2);
    ASSERT_EQ(ints.Receive(), 3);
  }

  TEST(ConcurrentReceivers, kLongTestOptions) {
    Channel<int64_t> ints;

    StaticThreadPool pool{4};

    static const size_t kFibers = 12;

    std::atomic<int64_t> received{0};

    // Receivers

    for (size_t i = 0; i < kFibers; ++i) {
      Spawn(pool, [&]() {
        while (true) {
          int value = ints.Receive();
          if (value == -1) {
            break;
          }
          received.fetch_add(value);
        }
      });
    }

    // Sender

    static const int kItems = 100000;

    int64_t sent = 0;

    Spawn(pool, [&]() {
      for (int i = 0; i < kItems; ++i) {
        ints.Send(i);
        sent += i;
      }

      // Poison pills
      for (size_t i = 0; i < kFibers; ++i) {
        ints.Send(-1);
      }
    });

    pool.Join();

    ASSERT_EQ(sent, received.load());
  }

  void TestConcurrentImpl(size_t senders, size_t receivers, size_t messages,
                          size_t capacity) {
    StaticThreadPool pool{5};

    Channel<int64_t> ints(capacity);

    std::atomic<int64_t> received{0};
    std::atomic<int64_t> sent{0};

    std::atomic<size_t> barrier{0};

    for (size_t i = 0; i < receivers; ++i) {
      Spawn(pool, [&]() {
        // One-pass barrier
        barrier.fetch_add(1);
        while (barrier != senders + receivers) {
          Yield();
        }

        while (true) {
          int value = ints.Receive();
          if (value == -1) {
            break;
          }
          received.fetch_add(value);
        }
      });
    }

    std::atomic<size_t> senders_done{0};

    for (size_t j = 0; j < senders; ++j) {
      Spawn(pool, [&]() {
        // One-pass barrier
        barrier.fetch_add(1);
        while (barrier != senders + receivers) {
          Yield();
        }

        for (size_t k = 1; k <= messages; ++k) {
          ints.Send(static_cast<int64_t>(k));
          sent.fetch_add(k);
        }
        if (++senders_done == senders) {
          // Last sender: send poison pills to receivers
          for (size_t i = 0; i < receivers; ++i) {
            ints.Send(-1);
          }
        }
      });
    }

    pool.Join();

    ASSERT_EQ(senders_done.load(), senders);
    ASSERT_EQ(sent.load(), received.load());
  }

  TEST(ConcurrentUnbounded1, kLongTestOptions) {
    TestConcurrentImpl(5, 5, 20'000, std::numeric_limits<size_t>::max());
  }

  TEST(ConcurrentUnbounded2, kLongTestOptions) {
    TestConcurrentImpl(2, 8, 20'000, std::numeric_limits<size_t>::max());
  }

  TEST(ConcurrentUnbounded3, kLongTestOptions) {
    TestConcurrentImpl(8, 2, 20'000, std::numeric_limits<size_t>::max());
  }

  TEST(ConcurrentBounded1, kLongTestOptions) {
    TestConcurrentImpl(5, 5, 100'000, 10);
  }

  TEST(ConcurrentBounded2, kLongTestOptions) {
    TestConcurrentImpl(2, 6, 100'000, 10);
  }

  TEST(ConcurrentBounded3, kLongTestOptions) {
    TestConcurrentImpl(6, 2, 100'000, 10);
  }

  TEST(ConcurrentNoBuffer1, kLongTestOptions) {
    // Unbounded channel
    TestConcurrentImpl(4, 4, 50'000, 1);
  }

  TEST(ConcurrentNoBuffer2, kLongTestOptions) {
    TestConcurrentImpl(2, 6, 50'000, 1);
  }

  TEST(ConcurrentNoBuffer3, kLongTestOptions) {
    TestConcurrentImpl(6, 2, 50'000, 1);
  }
}

TEST_SUITE(Select) {
  SIMPLE_TEST(JustWorks) {
    StaticThreadPool pool{4};

    Channel<int> ints;
    Channel<std::string> strs;

    Spawn(pool, [&]() {
      for (size_t i = 0; i < 5; ++i) {
        auto value = Select(ints, strs);
        switch (value.index()) {
          case 0:
            std::cout << "Received int " << std::get<0>(value) << std::endl;
            break;
          case 1:
            std::cout << "Received str " << std::get<1>(value) << std::endl;
            break;
        }
      }
    });

    Spawn(pool, [&]() {
      strs.Send("Hello!");
      std::this_thread::sleep_for(100ms);
      ints.Send(1);
      std::this_thread::sleep_for(100ms);
      strs.Send("Test");
      std::this_thread::sleep_for(100ms);
      ints.Send(42);
      std::this_thread::sleep_for(100ms);
      ints.Send(101);
    });

    pool.Join();
  }

  SIMPLE_TEST(ConcurrentSelects) {
    std::atomic<size_t> xs_consumed{0};
    std::atomic<size_t> ys_consumed{0};

    Channel<int> xs;
    Channel<int> ys;

    StaticThreadPool pool{4};

    static const size_t kSends = 100500;

    static const size_t kFibers = 16;

    for (size_t i = 0; i < kFibers; ++i) {
      Spawn(pool, [&]() {
        bool xs_done = false;
        bool ys_done = false;

        size_t iter = 0;

        while (!xs_done || !ys_done) {
          if (++iter % 17 == 0) {
            Yield();
          }
          auto value = Select(xs, ys);
          switch (value.index()) {
            case 0: {
              int x = std::get<0>(value);
              if (x == -1) {
                xs_done = true;
                xs.Send(-1);
              } else {
                xs_consumed.fetch_add(x);
              }
              break;
            }
            case 1:
              int y = std::get<1>(value);
              if (y == -1) {
                ys_done = true;
                ys.Send(-1);
              } else {
                ys_consumed.fetch_add(y);
              }
              break;
          }
        }
      });
    }

    Spawn(pool, [&]() {
      for (size_t i = 0; i < kSends; ++i) {
        xs.Send(i);
      }
      xs.Send(-1);
    });

    Spawn(pool, [&]() {
      for (size_t j = 0; j < kSends; ++j) {
        ys.Send(j);
      }
      ys.Send(-1);
    });

    pool.Join();

    static const size_t kProduced = kSends * (0 + kSends - 1) / 2;

    std::cout << "Xs consumed: " << xs_consumed.load() << std::endl;
    std::cout << "Ys consumed: " << ys_consumed.load() << std::endl;
    std::cout << "Expected: " << kProduced << std::endl;

    ASSERT_EQ(xs_consumed.load(), kProduced);
    ASSERT_EQ(ys_consumed.load(), kProduced);
  }

  SIMPLE_FIBER_TEST(SelectFairness, 1) {
    static const size_t kIterations = 10000;

    Channel<int> xs;
    Channel<int> ys;

    xs.Send(1);
    ys.Send(2);

    int balance = 0;

    for (size_t i = 0; i < kIterations; ++i) {
      auto value = Select(xs, ys);
      switch (value.index()) {
        case 0:
          ASSERT_EQ(std::get<0>(value), 1);
          xs.Send(1);
          ++balance;
          break;
        case 1:
          ASSERT_EQ(std::get<1>(value), 2);
          ys.Send(2);
          --balance;
          break;
      }
    }

    ASSERT_TRUE(std::abs(balance) < 1000);
  }
}
