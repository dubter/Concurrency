#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <twist/test/test.hpp>

#include <exe/executors/thread_pool.hpp>
#include <exe/executors/strand.hpp>

#include <exe/futures/core/future.hpp>
#include <exe/futures/combine/all.hpp>
#include <exe/futures/combine/first_of.hpp>
#include <exe/futures/util/execute.hpp>
#include <exe/futures/util/get.hpp>

#include <wheels/core/unit.hpp>

#include <atomic>
#include <chrono>

using namespace exe;

using wheels::Unit;

//////////////////////////////////////////////////////////////////////

void StressTestSubscribe() {
  executors::ThreadPool pool{4};

  while (wheels::test::KeepRunning()) {
    auto [f, p] = futures::MakeContract<std::string>();

    bool done = false;

    executors::Execute(pool, [p = std::move(p)]() mutable {
      std::move(p).SetValue("Hi");
    });

    executors::Execute(pool, [&done, f = std::move(f)]() mutable {
      std::move(f).Subscribe([&done](wheels::Result<std::string> message) {
        ASSERT_EQ(message.ExpectValue(), "Hi");
        done = true;
      });
    });

    pool.WaitIdle();

    ASSERT_TRUE(done);
  }

  pool.Stop();
}

//////////////////////////////////////////////////////////////////////

void StressTestPipeline() {
  executors::ThreadPool pool{4};
  executors::Strand strand{pool};

  size_t iter = 0;

  while (wheels::test::KeepRunning()) {
    ++iter;

    size_t pipelines = 1 + iter % 3;

    std::atomic<size_t> counter1{0};
    size_t counter2 = 0;
    std::atomic<size_t> counter3{0};

    for (size_t j = 0; j < pipelines; ++j) {
      futures::Execute(pool, [&]() -> Unit {
        ++counter1;
        return Unit{};
      }).Via(strand).Then([&](Unit) -> Unit {
        ++counter2;
        return Unit{};
      }).Via(pool).Subscribe([&](wheels::Result<Unit>) {
        ++counter3;
      });
    }

    pool.WaitIdle();

    ASSERT_EQ(counter1.load(), pipelines);
    ASSERT_EQ(counter2, pipelines);
    ASSERT_EQ(counter3.load(), pipelines);
  }

  pool.Stop();
}

//////////////////////////////////////////////////////////////////////

void StressTestAll() {
  executors::ThreadPool pool{4};

  size_t iter = 0;

  while (wheels::test::KeepRunning()) {
    ++iter;

    size_t inputs = 1 + iter % 4;

    std::vector<futures::Future<int>> futs;

    for (int j = 0; j < (int)inputs; ++j) {
      futs.push_back(futures::Execute(pool, [j]() -> int {
        return j;
      }));
    }

    auto all = futures::All(std::move(futs));

    auto ints = futures::GetResult(std::move(all)).ExpectValue();

    ASSERT_EQ(ints.size(), inputs);
    for (int j = 0; j < (int)inputs; ++j) {
      ASSERT_EQ(ints[j], j);
    }
  }

  pool.Stop();
}

//////////////////////////////////////////////////////////////////////

void StressTestFirstOf() {
  executors::ThreadPool pool{4};

  size_t iter = 0;

  while (wheels::test::KeepRunning()) {
    ++iter;

    size_t inputs = 1 + iter % 4;

    std::vector<futures::Future<int>> futs;

    for (int j = 0; j < (int)inputs; ++j) {
      futs.push_back(futures::Execute(pool, [j]() -> int {
        return j;
      }));
    }

    auto first_of = futures::FirstOf(std::move(futs));

    auto first_value = futures::GetResult(std::move(first_of)).ExpectValue();

    ASSERT_TRUE((first_value >= 0) && (first_value <= 4));

    pool.WaitIdle();
  }

  pool.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Futures) {
  TWIST_TEST_TL(StressSubscribe, 5s) {
    StressTestSubscribe();
  }

  TWIST_TEST_TL(StressPipeline, 5s) {
    StressTestPipeline();
  }

  TWIST_TEST_TL(StressAll, 5s) {
    StressTestAll();
  }

  TWIST_TEST_TL(StressFirstOf, 5s) {
    StressTestFirstOf();
  }
}

RUN_ALL_TESTS()
