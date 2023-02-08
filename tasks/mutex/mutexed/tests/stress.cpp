#include "mutexed.hpp"

#include <twist/test/test.hpp>

#include <twist/test/util/race.hpp>
#include <twist/test/util/plate.hpp>

#include <wheels/test/util.hpp>

TEST_SUITE(Mutexed) {
  TWIST_TEST_TL(Stress, 3s) {
    static const size_t kThreads = 3;

    // Set of hungry threads
    twist::test::Race race;
    // Plate shared between threads
    Mutexed<twist::test::Plate> plate;

    for (size_t i = 0; i < kThreads; ++i) {
      race.Add([&]() {
        while (wheels::test::KeepRunning()) {
          Acquire(plate)->Access();
        }
      });
    }

    race.Run();
  }
}

RUN_ALL_TESTS()
