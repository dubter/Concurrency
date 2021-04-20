#include <wheels/test/test_framework.hpp>

#include <await/futures/core/promise.hpp>

#include <await/futures/util/async.hpp>
#include <await/futures/util/after.hpp>

#include <await/futures/combine/all.hpp>
#include <await/futures/combine/first_of.hpp>

#include <await/executors/static_thread_pool.hpp>
#include <await/executors/label_thread.hpp>

#include <await/time/time_keeper.hpp>

#include "../helpers.hpp"

using namespace std::chrono_literals;
using namespace await::futures;
using namespace await::executors;
using wheels::Result;
using wheels::Status;
using wheels::Unit;

////////////////////////////////////////////////////////////////////////////////

// Helpers

template <typename T>
Future<T> AsyncError(wheels::Duration d) {
  return After(d).Then([](Unit) -> T {
    throw std::runtime_error("fail");
  });
}

template <typename T>
Future<T> AsyncValue(T value, wheels::Duration d) {
  return After(d).Then([value](Unit) {
    return value;
  });
}

////////////////////////////////////////////////////////////////////////////////

TEST_SUITE(Futures) {
  SIMPLE_TEST(JustWorks) {
    Promise<int> p;
    auto f = p.MakeFuture();

    ASSERT_FALSE(f.IsReady());
    ASSERT_TRUE(f.IsValid());

    std::move(p).SetValue(42);

    ASSERT_TRUE(f.IsReady());
    ASSERT_EQ(std::move(f).GetResult(), 42);
    ASSERT_FALSE(f.IsValid());
  }

  SIMPLE_TEST(MakeContract) {
    auto [f, p] = MakeContract<int>();

    ASSERT_FALSE(f.IsReady());

    std::move(p).SetValue(42);

    ASSERT_TRUE(f.IsReady());
    ASSERT_EQ(std::move(f).GetResult(), 42);
  }

  SIMPLE_TEST(BlockingGetResult) {
    static const std::string kMessage = "Hello, world!";

    auto tp = MakeStaticThreadPool(1, "tp");

    auto [f, p] = MakeContract<std::string>();
    tp->Execute([p = std::move(p)]() mutable {
      std::this_thread::sleep_for(1s);
      std::move(p).SetValue(kMessage);
    });

    {
      test_helpers::CPUTimeBudgetGuard cpu_time_budget(100ms);
      auto result = std::move(f).GetResult();

      ASSERT_TRUE(result.IsOk());
      ASSERT_EQ(*result, kMessage);
    }

    tp->Join();
  }

  SIMPLE_TEST(Exception) {
    auto [f, p] = MakeContract<std::string>();

    try {
      throw std::runtime_error("test");
    } catch (...) {
      std::move(p).SetError(std::current_exception());
    }

    ASSERT_THROW(std::move(f).GetValue(), std::runtime_error);
  }

  SIMPLE_TEST(Completed) {
    auto f = Future<int>::Completed(7);
    ASSERT_TRUE(f.IsValid());
    auto result = std::move(f).GetResult();
    ASSERT_TRUE(result.IsOk());
    ASSERT_EQ(*result, 7);
  }

  std::error_code MakeTimedOutErrorCode() {
    return std::make_error_code(std::errc::timed_out);
  }

  SIMPLE_TEST(Failed) {
    auto f = Future<int>::Failed(MakeTimedOutErrorCode());
    ASSERT_TRUE(f.IsValid());
    auto result = std::move(f).GetResult();
    ASSERT_FALSE(result.IsOk());
  }

  SIMPLE_TEST(Invalid) {
    auto f = Future<int>::Invalid();
    ASSERT_FALSE(f.IsValid());
  }

  SIMPLE_TEST(AsyncVia) {
    auto tp = MakeStaticThreadPool(3, "tp");

    {
      auto good = []() -> std::string {
        ExpectThread("tp");
        return "Hello!";
      };

      auto f = AsyncVia(tp, good);
      ASSERT_EQ(std::move(f).GetValue(), "Hello!");
    }

    {
      auto bad = []() -> int {
        ExpectThread("tp");
        throw std::logic_error("test");
      };

      auto result = AsyncVia(tp, bad).GetResult();
      ASSERT_TRUE(result.HasError());
      ASSERT_THROW(result.ThrowIfError(), std::logic_error);
    }

    tp->Join();
  }

  // Subscribe

  SIMPLE_TEST(Subscribe1) {
    auto [f, p] = MakeContract<int>();

    std::move(p).SetValue(17);

    bool called = false;

    std::move(f).Subscribe([&called](Result<int> result) {
      ASSERT_EQ(result.ValueOrThrow(), 17);
      called = true;
    });

    ASSERT_FALSE(f.IsValid());
    ASSERT_TRUE(called);
  }

  SIMPLE_TEST(Subscribe2) {
    auto [f, p] = MakeContract<int>();

    auto result = wheels::make_result::Throw<std::runtime_error>("test");
    std::move(p).Set(std::move(result));

    bool called = false;

    std::move(f).Subscribe([&called](Result<int> result) {
      ASSERT_TRUE(result.HasError());
      called = true;
    });

    ASSERT_TRUE(called);
  }

  SIMPLE_TEST(Subscribe3) {
    auto tp = MakeStaticThreadPool(1, "tp");

    auto [f, p] = MakeContract<std::string>();

    std::atomic<bool> called{false};

    std::move(f).Subscribe([&](Result<std::string> result) {
      ExpectThread("tp");
      ASSERT_EQ(result.ValueOrThrow(), "Hello!");
      called.store(true);
    });

    ASSERT_FALSE(f.IsValid());
    ASSERT_FALSE(called.load());

    tp->Execute([p = std::move(p)]() mutable {
      std::move(p).SetValue("Hello!");
    });

    tp->Join();

    ASSERT_TRUE(called.load());
  }

  SIMPLE_TEST(SubscribeVia1) {
    test_helpers::CPUTimeBudgetGuard cpu_time_budget(100ms);

    auto tp = MakeStaticThreadPool(1, "callbacks");

    auto [f, p] = MakeContract<int>();

    std::move(p).SetValue(17);

    std::atomic<bool> called = false;

    auto callback = [&called](Result<int> result) mutable {
      ExpectThread("callbacks");
      ASSERT_EQ(result.ValueOrThrow(), 17);
      called.store(true);
    };

    // Schedule to thread pool immediately
    std::move(f).Via(tp).Subscribe(callback);

    tp->Join();

    ASSERT_TRUE(called);
  }

  SIMPLE_TEST(SubscribeVia2) {
    test_helpers::CPUTimeBudgetGuard cpu_time_budget(100ms);

    auto tp1 = MakeStaticThreadPool(1, "tp1");
    auto tp2 = MakeStaticThreadPool(1, "tp2");

    auto [f, p] = MakeContract<int>();

    std::atomic<bool> called = false;

    auto callback = [&called](Result<int> result) mutable {
      ExpectThread("tp2");
      ASSERT_EQ(result.ValueOrThrow(), 42);
      called.store(true);
    };

    std::move(f).Via(tp2).Subscribe(callback);

    tp1->Execute([p = std::move(p)]() mutable {
      ExpectThread("tp1");
      std::this_thread::sleep_for(1s);
      std::move(p).SetValue(42);
    });

    tp1->Join();
    tp2->Join();

    ASSERT_TRUE(called);
  }

  // ???
  SIMPLE_TEST(ViaDoesNotBlockThreadPool) {
    auto tp = MakeStaticThreadPool(1, "single");

    auto [f, p] = MakeContract<int>();

    test_helpers::OneShotEvent done;

    auto set_done = [&done](Result<int> result) {
      ExpectThread("single");
      ASSERT_EQ(result.ValueOrThrow(), 42);
      done.Set();
    };

    std::move(f).Via(tp).Subscribe(set_done);

    // Thread pool is idle
    AsyncVia(tp, []() -> Unit {
      return {};
    }).GetValue();

    std::move(p).SetValue(42);
    done.Await();

    tp->Join();
  }

  SIMPLE_TEST(Then) {
    auto [f, p] = MakeContract<int>();

    auto g = std::move(f).Then([](int v) {
      return v * 2 + 1;
    });
    std::move(p).SetValue(3);
    ASSERT_EQ(std::move(g).GetValue(), 7);
  }

  SIMPLE_TEST(ThenThreadPool) {
    auto tp = MakeStaticThreadPool(4, "test");

    auto compute = []() -> int {
      std::this_thread::sleep_for(1s);
      return 42;
    };

    auto process = [](int v) -> int {
      return v + 1;
    };

    Future<int> f1 = AsyncVia(tp, compute);

    Future<int> f2 = std::move(f1).Then(process);

    ASSERT_EQ(std::move(f2).GetValue(), 43);

    tp->Join();
  }

  SIMPLE_TEST(ViaThen) {
    auto tp = MakeStaticThreadPool(2, "test");

    auto [f, p] = MakeContract<Unit>();

    auto g = std::move(f).Via(tp).Then([](Unit) {
      ExpectThread("test");
      return 42;
    });

    // Launch
    std::move(p).SetValue({});

    ASSERT_EQ(std::move(g).GetValue(), 42);

    tp->Join();
  }

  SIMPLE_TEST(ThenExecutor) {
    auto tp = MakeStaticThreadPool(1, "test");
    auto [f, p] = MakeContract<Unit>();
    auto g = std::move(f).Via(tp).Then([](Unit) {
      return 42;
    });
    ASSERT_EQ(g.GetExecutor(), tp);
  }

  SIMPLE_TEST(Pipeline) {
    auto tp = MakeStaticThreadPool(4, "tp");

    // Pipeline stages:

    auto first = []() -> int {
      return 42;
    };

    auto second = [](int value) {
      return value * 2;
    };

    auto third = [](int value) {
      return value + 1;
    };

    auto f = AsyncVia(tp, first).Then(second).Then(third);

    ASSERT_EQ(std::move(f).GetValue(), 42 * 2 + 1);

    tp->Join();
  }

  SIMPLE_TEST(Errors1) {
    auto tp = MakeStaticThreadPool(4, "tp");

    // Pipeline stages:

    auto first = []() -> int {
      return 1;
    };

    auto second = [](int v) -> int {
      std::cout << v << " x 2" << std::endl;
      return v * 2;
    };

    auto third = [](int v) -> int {
      std::cout << v << " + 1" << std::endl;
      return v + 1;
    };

    auto error_handler = [](wheels::Error) -> int {
      return 42;
    };

    auto last = [](int v) -> int {
      ExpectThread("tp");
      return v + 11;
    };

    auto f = AsyncVia(tp, first)
                 .Then(second)
                 .Then(third)
                 .Recover(error_handler)
                 .Then(last);

    ASSERT_EQ(std::move(f).GetValue(), 14);

    tp->Join();
  }

  SIMPLE_TEST(Errors2) {
    auto tp = MakeStaticThreadPool(4, "tp");

    // Pipeline stages:

    auto first = []() -> int {
      throw std::runtime_error("first");
    };

    auto second = [](int v) -> int {
      std::cout << v << " x 2" << std::endl;
      return v * 2;
    };

    auto third = [](int v) -> int {
      std::cout << v << " + 1" << std::endl;
      return v + 1;
    };

    auto error_handler = [](wheels::Error) -> int {
      return 42;
    };

    auto last = [](int v) -> int {
      ExpectThread("tp");
      return v + 11;
    };

    auto pipeline = AsyncVia(tp, first)
                        .Then(second)
                        .Then(third)
                        .Recover(error_handler)
                        .Then(last);

    ASSERT_EQ(std::move(pipeline).GetValue(), 53);

    tp->Join();
  }

  SIMPLE_TEST(AsyncThenAfter) {
    test_helpers::CPUTimeBudgetGuard cpu_time_budget(100ms);

    auto [f, p] = MakeContract<Unit>();

    std::atomic<bool> done{false};

    auto pipeline = std::move(f)
                        .Then([](Unit) {
                          return After(1s);
                        })
                        .Then([](Unit) {
                          return After(500ms);
                        })
                        .Then([](Unit) {
                          return After(250ms);
                        })
                        .Then([&done](Unit) -> Unit {
                          done = true;
                          std::cout << "Finally!" << std::endl;
                          return {};
                        });

    // Launch
    std::move(p).SetValue({});

    std::this_thread::sleep_for(1250ms);
    ASSERT_FALSE(done);

    std::move(pipeline).GetValue();
    ASSERT_TRUE(done);
  }

  class Calculator {
   public:
    Calculator(IThreadPoolPtr tp) : tp_(tp) {
    }

    Future<int> Increment(int value) {
      return AsyncVia(tp_, [value]() {
        std::this_thread::sleep_for(1s);
        return value + 1;
      });
    }

    Future<int> Double(int value) {
      return AsyncVia(tp_, [value]() {
        std::this_thread::sleep_for(1s);
        return value * 2;
      });
    }

   private:
    IThreadPoolPtr tp_;
  };

  SIMPLE_TEST(AsyncCalculator) {
    test_helpers::CPUTimeBudgetGuard cpu_time_budget(100ms);

    auto tp = MakeStaticThreadPool(4, "tp");

    Calculator calculator(tp);

    auto pipeline = calculator.Increment(1)
                        .Then([&](int value) {
                          return calculator.Double(value);
                        })
                        .Then([&](int value) {
                          return calculator.Increment(value);
                        });

    ASSERT_EQ(std::move(pipeline).GetValue(), 5);

    tp->Join();
  }

  SIMPLE_TEST(Pipeline2) {
    test_helpers::CPUTimeBudgetGuard cpu_time_budget(100ms);

    auto tp1 = MakeStaticThreadPool(2, "tp1");
    auto tp2 = MakeStaticThreadPool(3, "tp2");

    auto [f, p] = MakeContract<std::string>();

    auto make_stage = [](int index, std::string label) {
      return [index, label](std::string path) {
        ExpectThread(label);
        std::cout << "At stage " << index << std::endl;
        return path + "->" + std::to_string(index);
      };
    };

    auto almost_there = std::move(f)
                            .Via(tp1)
                            .Then(make_stage(1, "tp1"))
                            .Then(make_stage(2, "tp1"))
                            .Via(tp2)
                            .Then(make_stage(3, "tp2"));

    std::move(p).SetValue("start");
    std::this_thread::sleep_for(100ms);

    auto finally = std::move(almost_there)
                       .Then(make_stage(4, "tp2"))
                       .Via(tp1)
                       .Then(make_stage(5, "tp1"));

    ASSERT_EQ(std::move(finally).GetValue(), "start->1->2->3->4->5");

    tp1->Join();
    tp2->Join();
  }

  // Combinators

  // All

  SIMPLE_TEST(All) {
    std::vector<Promise<int>> promises{3};

    std::vector<Future<int>> futures;
    for (auto& p : promises) {
      futures.push_back(p.MakeFuture());
    }

    auto all = All(std::move(futures));

    ASSERT_FALSE(all.IsReady());

    std::move(promises[2]).SetValue(7);
    std::move(promises[0]).SetValue(3);

    // Still not completed
    ASSERT_FALSE(all.IsReady());

    std::move(promises[1]).SetValue(5);

    ASSERT_TRUE(all.IsReady());

    Result<std::vector<int>> ints = std::move(all).GetResult();
    ASSERT_TRUE(ints.IsOk());

    ASSERT_EQ(*ints, std::vector<int>({7, 3, 5}));
  }

  SIMPLE_TEST(AllFails) {
    std::vector<Promise<int>> promises{3};

    std::vector<Future<int>> futures;
    for (auto& p : promises) {
      futures.push_back(p.MakeFuture());
    }

    auto all = All(std::move(futures));

    ASSERT_FALSE(all.IsReady());

    // First error
    std::move(promises[1]).SetError(test_helpers::MakeTestError());
    ASSERT_TRUE(all.IsReady());

    // Second error
    std::move(promises[0]).SetError(test_helpers::MakeTestError());

    auto all_result = std::move(all).GetResult();
    ASSERT_TRUE(all_result.HasError());
  }

  SIMPLE_TEST(AllEmptyInput) {
    auto all = All(std::vector<Future<int>>{});

    ASSERT_TRUE(all.IsReady());
    ASSERT_TRUE(std::move(all).GetResult().IsOk());
  }

  SIMPLE_TEST(AllMultiThreaded) {
    auto tp = MakeStaticThreadPool(4, "tp");

    auto async_value = [tp](int value) {
      return AsyncVia(tp, [value]() {
        ExpectThread("tp");
        std::this_thread::sleep_for(100ms);
        return value;
      });
    };

    static const size_t kValues = 16;

    std::vector<Future<int>> fs;
    for (size_t i = 0; i < kValues; ++i) {
      fs.push_back(async_value((int)i));
    }

    auto ints = All(std::move(fs)).GetValue();
    std::sort(ints.begin(), ints.end());

    ASSERT_EQ(ints.size(), kValues);
    for (int i = 0; i < (int)kValues; ++i) {
      ASSERT_EQ(ints[i], i);
    }

    tp->Join();
  }

  // FirstOf

  SIMPLE_TEST(FirstOf) {
    test_helpers::CPUTimeBudgetGuard cpu_time_budget(100ms);
    test_helpers::WallTimeLimitGuard wall_time_limit(1200ms);

    auto first_of = FirstOf(
        AsyncValue<int>(1, 2s),
        AsyncValue<int>(2, 1s),
        AsyncValue<int>(3, 3s));

    ASSERT_EQ(FirstOf(std::move(first_of)).GetValue(), 2);
  }

  SIMPLE_TEST(FirstOfWithErrors1) {
    std::vector<Future<int>> fs;

    auto first_of = FirstOf(AsyncError<int>(500ms), AsyncValue(13, 2s),
                            AsyncError<int>(1500ms), AsyncValue(42, 1s));

    ASSERT_EQ(std::move(first_of).GetValue(), 42);
  }

  SIMPLE_TEST(FirstOfWithErrors2) {
    // TODO
  }

  SIMPLE_TEST(FirstOfEmptyInput) {
    std::vector<Future<int>> inputs;
    auto first_of = FirstOf(std::move(inputs));
    ASSERT_FALSE(first_of.IsValid());
  }

  SIMPLE_TEST(FirstOfDontWaitAfterValue) {
    auto first_of = FirstOf(AsyncValue(1, 20s), AsyncValue(2, 500ms));
    ASSERT_EQ(std::move(first_of).GetValue(), 2);
  }

  SIMPLE_TEST(Via) {
    auto tp = MakeStaticThreadPool(2, "test");

    auto [f, p] = MakeContract<Unit>();

    auto answer = std::move(f).Via(tp).Then([](Unit) {
      ExpectThread("test");
      return 42;
    });

    std::move(p).SetValue({});
    ASSERT_EQ(std::move(answer).GetResult().ValueOrThrow(), 42);

    tp->Join();
  }

  SIMPLE_TEST(AsyncVia2) {
    auto tp = MakeStaticThreadPool(4, "test");

    auto f = AsyncVia(tp, []() {
      ExpectThread("test");
      return 42;
    });

    std::this_thread::sleep_for(1s);

    std::move(f).Subscribe([](Result<int>) {
      ExpectThread("test");
    });

    tp->Join();
  }
}
