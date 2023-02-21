#include <futures/promise.hpp>

#include <twist/ed/stdlike/thread.hpp>

#include <twist/test/with/wheels/stress.hpp>
#include <twist/test/race.hpp>

#include <string>

using namespace std::chrono_literals;

using stdlike::Promise;
using stdlike::Future;

TEST_SUITE(Future) {
  TWIST_TEST_REPEAT(WaitForValue, 10s) {
    // Make contract

    Promise<int> p;
    auto f = p.MakeFuture();

    // Run concurrent producer & consumer

    twist::test::Race race;

    race.Add([&p]() {
      p.SetValue(17);
    });

    race.Add([&f]() {
      ASSERT_EQ(f.Get(), 17);
    });

    race.Run();
  }

  TWIST_TEST_REPEAT(WaitForValue2, 10s) {
    // Contracts

    Promise<std::string> p0;
    Promise<std::string> p1;

    auto f0 = p0.MakeFuture();
    auto f1 = p1.MakeFuture();

    // Race

    twist::test::Race race;

    // Producers

    race.Add([&p0]() {
      p0.SetValue("Hello");
    });

    race.Add([&p1]() {
      p1.SetValue("World");
    });

    // Consumers

    race.Add([&f0]() {
      ASSERT_EQ(f0.Get(), "Hello");
    });

    race.Add([&f1]() {
      ASSERT_EQ(f1.Get(), "World");
    });

    race.Run();
  }

  template <typename T>
  void Drop(T value) {
    (void)value;
  }

  TWIST_TEST_REPEAT(SharedState, 5s) {
    // Make contract

    Promise<std::string> p;
    Future<std::string> f = p.MakeFuture();

    // Run concurrent producer & consumer

    twist::test::Race race;

    race.Add([f = std::move(f)]() mutable {
      if (wheels::test::TestIteration() % 2 == 1) {
        ASSERT_EQ(f.Get(), "Test");
      }
      Drop(std::move(f));
    });

    race.Add([p = std::move(p)]() mutable {
      p.SetValue("Test");
      Drop(std::move(p));
    });

    race.Run();
  }
}

RUN_ALL_TESTS()
