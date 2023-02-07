#include "../cyclic_barrier.hpp"

#include <twist/test/test.hpp>
#include <twist/test/runs.hpp>
#include <twist/test/util/race.hpp>

#include <wheels/test/util.hpp>

#include <vector>

////////////////////////////////////////////////////////////////////////////////

namespace leader {

void Test(const size_t threads, size_t iterations) {
  CyclicBarrier barrier{threads};
  size_t leader = 0;

  twist::test::util::Race race;

  for (size_t i = 0; i < threads; ++i) {
    race.Add([&, i]() {
      barrier.ArriveAndWait();

      for (size_t k = 0; k < iterations; ++k) {
        // Rotating leader writes to shared variable
        if (k % threads == i) {
          leader = k;
        } else {
          twist::rt::strand::stdlike::this_thread::yield();
        }

        barrier.ArriveAndWait();

        // All threads read from shared variable
        ASSERT_EQ(leader, k);

        barrier.ArriveAndWait();
      }
    });
  };

  race.Run();
}

}  // namespace leader

TWIST_TEST_RUNS(RotatingLeader, leader::Test)
    ->TimeLimit(30s)
    ->Run(2, 50'000)
    ->Run(5, 25'000)
    ->Run(10, 10'000);

#if defined(TWIST_FIBERS)

TWIST_TEST_RUNS(RotatingLeaderExt, leader::Test)
    ->TimeLimit(30s)
    ->Run(10, 100'000);

#endif

////////////////////////////////////////////////////////////////////////////////

namespace rotate {
void Test(size_t threads, size_t iterations) {
  CyclicBarrier barrier_{threads};
  std::vector<size_t> vector_(threads);

  twist::test::util::Race race;

  for (size_t t = 0; t < threads; ++t) {
    race.Add([&, t]() {
      // Setup

      vector_[t] = t;
      barrier_.ArriveAndWait();

      // Rotate

      for (size_t i = 0; i < iterations; ++i) {
        // Choose slot to move
        size_t slot = (t + i) % threads;
        size_t prev_slot = slot > 0 ? (slot - 1) : (threads - 1);

        // Move value from slot to prev_slot
        auto value = vector_[slot];
        barrier_.ArriveAndWait();
        vector_[prev_slot] = value;
        barrier_.ArriveAndWait();
      }

      ASSERT_EQ(vector_[t], (t + iterations) % threads);
    });
  }

  race.Run();
}
}  // namespace rotate

TWIST_TEST_RUNS(RotateVector, rotate::Test)
    ->TimeLimit(30s)
    ->Run(2, 50'001)
    ->Run(5, 50'007)
    ->Run(10, 25'011)
    ->Run(15, 10'007);

////////////////////////////////////////////////////////////////////////////////

RUN_ALL_TESTS()
