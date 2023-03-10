#include "../wait_group.hpp"

#include <twist/test/with/wheels/stress.hpp>

#include <twist/test/race.hpp>

void StressTest(size_t workers, size_t waiters) {
  WaitGroup wg;

  std::atomic<size_t> work{0};

  twist::test::Race race;

  wg.Add(workers);

  for (size_t i = 0; i < waiters; ++i) {
    race.Add([&] {
      wg.Wait();
      ASSERT_EQ(work.load(), workers);
    });
  }

  for (size_t i = 1; i <= workers; ++i) {
    race.Add([&] {
      ++work;
      wg.Done();
    });
  }

  race.Run();
}

TEST_SUITE(WaitGroup) {
  TWIST_TEST_REPEAT(Run_1_1, 5s) {
    StressTest(1, 1);
  }

  TWIST_TEST_REPEAT(Run_2_2, 5s) {
    StressTest(2, 2);
  }

  TWIST_TEST_REPEAT(Run_3_2, 5s) {
    StressTest(3, 2);
  }

  TWIST_TEST_REPEAT(Run_4_4, 5s) {
    StressTest(4, 4);
  }
}

RUN_ALL_TESTS();
