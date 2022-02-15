#include "../queue_spinlock.hpp"

#include <twist/test/test.hpp>

#include <twist/test/util/race.hpp>
#include <twist/test/util/plate.hpp>

#include <wheels/test/util.hpp>

#include <chrono>

using spinlocks::QueueSpinLock;
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////////////////////////

TEST_SUITE(Mutex) {
  void Test(size_t threads) {
    twist::test::util::Plate plate;  // Guarded by mutex
    QueueSpinLock qspinlock;

    twist::test::util::Race race{threads};

    for (size_t i = 0; i < threads; ++i) {
      race.Add([&]() {
        while (wheels::test::KeepRunning()) {
          QueueSpinLock guard(qspinlock);
          {
            // Critical section
            plate.Access();
          }
        }
      });
    }

    race.Run();

    std::cout << "Critical sections: " << plate.AccessCount() << std::endl;
  }

  TWIST_TEST_TL(Stress1, 5s) {
    Test(2);
  }

  TWIST_TEST_TL(Stress2, 5s) {
    Test(5);
  }
}

////////////////////////////////////////////////////////////////////////////////

TEST_SUITE(MissedWakeup) {
  void Test(size_t threads) {
    QueueSpinLock qspinlock;

    twist::test::util::Race race{threads};

    for (size_t i = 0; i < threads; ++i) {
      race.Add([&]() {
        QueueSpinLock::Guard guard(qspinlock);
        // Critical section
      });
    };

    race.Run();
  }

  TWIST_ITERATE_TEST(Stress1, 5s) {
    Test(2);
  }

  TWIST_ITERATE_TEST(Stress2, 5s) {
    Test(3);
  }
}

////////////////////////////////////////////////////////////////////////////////

RUN_ALL_TESTS()
