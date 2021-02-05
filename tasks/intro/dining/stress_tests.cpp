#include "table.hpp"
#include "philosopher.hpp"

#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <twist/test/test.hpp>
#include <twist/test/inject_fault.hpp>
#include <twist/test/util/race.hpp>

#include <vector>

using namespace dining;

void Dining(size_t seats) {
  Table table_{seats};
  std::vector<Philosopher> philosophers_;

  for (size_t seat = 0; seat < seats; ++seat) {
    philosophers_.emplace_back(table_, seat);
  }

  twist::test::util::Race race{seats};

  for (size_t seat = 0; seat < seats; ++seat) {
    race.Add([&, seat]() {
      auto& philosopher = philosophers_.at(seat);
      while (wheels::test::KeepRunning()) {
        philosopher.EatThenThink();
      }
    });
  }

  race.Start();

  if (!twist::test::FaultsEnabled()) {
    // Check starvation

    std::vector<size_t> meals;
    for (auto& philosopher : philosophers_) {
      meals.push_back(philosopher.Meals());
    }

    size_t min_meals = *std::min_element(meals.begin(), meals.end());
    size_t max_meals = *std::max_element(meals.begin(), meals.end());

    std::cout << "Meals: min = " << min_meals << ", max = " << max_meals
              << std::endl;

    ASSERT_TRUE_M(min_meals * 10 > max_meals, "Starvation");
  }
}

TEST_SUITE(DiningPhilosophers) {
  TWIST_TEST_TL(Stress1, 3s) {
    Dining(/*seats=*/2);
  }

  TWIST_TEST_TL(Stress2, 3s) {
    Dining(/*seats=*/3);
  }

  TWIST_TEST_TL(Stress3, 5s) {
    Dining(/*seats=*/5);
  }

  TWIST_TEST_TL(Stress4, 10s) {
    Dining(/*seats=*/10);
  }
}

RUN_ALL_TESTS();
