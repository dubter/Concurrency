#include <exe/executors/thread_pool.hpp>

#include <exe/fibers/core/api.hpp>
#include <exe/fibers/sync/channel.hpp>
#include <exe/fibers/sync/wait_group.hpp>

#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <twist/test/test.hpp>

#include <random>

using namespace exe;

//////////////////////////////////////////////////////////////////////

void StressTest(size_t buffer, size_t senders, size_t receivers) {
  executors::ThreadPool scheduler{4};

  std::atomic<int64_t> checksum{0};
  std::atomic<size_t> sends{0};
  bool done = false;

  fibers::Go(scheduler, [&]() {
    fibers::Channel<uint64_t> ints{buffer};

    fibers::WaitGroup wg;
    wg.Add(senders + receivers);

    std::atomic<size_t> senders_left{senders};

    for (size_t i = 0; i < senders; ++i) {
      fibers::Go([&, i, ints]() mutable {
        std::mt19937 twister{(uint32_t)i};

        size_t iter = 0;

        while (wheels::test::KeepRunning()) {
          ++iter;

          int64_t value = twister();

          ints.Send(value);

          checksum.fetch_xor(value);
          ++sends;

          if (iter % 7 == 0) {
            fibers::self::Yield();
          }
        }

        if (senders_left.fetch_sub(1) == 1) {
          // Last sender
          for (size_t j = 0; j < receivers; ++j) {
            ints.Send(-1);  // Poison pill
          }
        }

        wg.Done();
      });
    }

    for (size_t j = 0; j < receivers; ++j) {
      fibers::Go([&, ints]() mutable {
        size_t iter = 0;

        while (true) {
          ++iter;

          int64_t value = ints.Receive();
          if (value == -1) {
            break;
          }
          checksum.fetch_xor(value);

          if (iter % 7 == 0) {
            fibers::self::Yield();
          }
        }

        wg.Done();
      });
    }

    wg.Wait();

    done = true;
  });

  scheduler.WaitIdle();

  ASSERT_TRUE(done);
  ASSERT_EQ(checksum.load(), 0);

  std::cout << "Sends: " << sends << std::endl;

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Channel) {
  TWIST_TEST_TL(Stress1, 5s) {
    StressTest(1, 2, 5);
  }

  TWIST_TEST_TL(Stress2, 5s) {
    StressTest(1, 5, 2);
  }

  TWIST_TEST_TL(Stress3, 5s) {
    StressTest(3, 5, 7);
  }

  TWIST_TEST_TL(Stress4, 5s) {
    StressTest(3, 8, 6);
  }

  TWIST_TEST_TL(Stress5, 5s) {
    StressTest(11, 15, 12);
  }

  TWIST_TEST_TL(Stress6, 5s) {
    StressTest(11, 14, 18);
  }
}

RUN_ALL_TESTS()
