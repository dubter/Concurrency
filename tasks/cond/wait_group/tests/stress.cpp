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

TWIST_TEST_TEMPLATE(WaitGroup, StressTest)
  ->TimeLimit(5s)
  ->Run(1, 1)
  ->Run(2, 2)
  ->Run(3, 2)
  ->Run(4, 4);

RUN_ALL_TESTS();
