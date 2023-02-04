#include <exe/futures/core/future.hpp>

#include <exe/futures/combine/all.hpp>
#include <exe/futures/combine/first_of.hpp>

#include <exe/futures/util/execute.hpp>
#include <exe/futures/util/get.hpp>

#include <exe/executors/manual.hpp>
#include <exe/executors/thread_pool.hpp>
#include <exe/executors/strand.hpp>

#include <wheels/test/framework.hpp>
#include <wheels/test/cpu_time.hpp>

#include <wheels/core/unit.hpp>

#include <thread>

using namespace exe;

using wheels::Unit;

//////////////////////////////////////////////////////////////////////

auto TimeoutError() {
  return std::make_error_code(std::errc::timed_out);
}

//////////////////////////////////////////////////////////////////////

struct MoveOnly {
  explicit MoveOnly(std::string data_) : data(data_) {
  }

  MoveOnly(const MoveOnly& that) = delete;
  MoveOnly& operator=(const MoveOnly& that) = delete;

  MoveOnly(MoveOnly&& that) = default;

  std::string data;
};

struct NonDefaultConstructable {
  explicit NonDefaultConstructable(int v) : value(v) {
  }

  int value{0};
};

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Futures) {
  SIMPLE_TEST(Invalid) {
    auto f = futures::Future<std::string>::Invalid();
    ASSERT_FALSE(f.IsValid());
  }

  SIMPLE_TEST(SetValue) {
    auto [f, p] = futures::MakeContract<std::string>();

    ASSERT_FALSE(f.IsReady());

    std::move(p).SetValue("Hello");

    ASSERT_TRUE(f.IsReady());

    auto result = std::move(f).GetReadyResult();
    ASSERT_TRUE(result.IsOk());
    ASSERT_EQ(*result, "Hello");
  }

  SIMPLE_TEST(SetMoveOnlyValue) {
    auto [f, p] = futures::MakeContract<MoveOnly>();

    ASSERT_FALSE(f.IsReady());

    std::move(p).SetValue(MoveOnly{"Hello"});

    ASSERT_TRUE(f.IsReady());

    auto result = std::move(f).GetReadyResult();
    ASSERT_TRUE(result.IsOk());
    ASSERT_EQ(result->data, "Hello");
  }

  SIMPLE_TEST(SetError) {
    auto [f, p] = futures::MakeContract<int>();

    ASSERT_FALSE(f.IsReady());

    std::move(p).SetError(TimeoutError());

    ASSERT_TRUE(f.IsReady());

    auto result = std::move(f).GetReadyResult();
    ASSERT_TRUE(result.HasError());

    auto err = result.GetErrorCode();
    std::cout << "Error = " << err.message();
  }

  SIMPLE_TEST(SetException) {
    auto [f, p] = futures::MakeContract<int>();

    try {
      throw std::runtime_error("Test");
    } catch (...) {
      std::move(p).SetError(std::current_exception());
    }

    auto result = std::move(f).GetReadyResult();

    ASSERT_TRUE(result.HasError());
    ASSERT_THROW(result.ValueOrThrow(), std::runtime_error);
  }

  SIMPLE_TEST(ExecuteValue) {
    executors::ManualExecutor manual;

    auto f = futures::Execute(manual, []() {
      return 7;
    });

    manual.RunNext();

    ASSERT_TRUE(f.IsReady());
    ASSERT_EQ(std::move(f).GetReadyResult().ExpectValue(), 7);
  }

  SIMPLE_TEST(ExecuteError) {
    executors::ManualExecutor manual;

    auto f = futures::Execute(manual, []() -> Unit {
      throw std::runtime_error("Test");
    });

    manual.RunNext();

    ASSERT_TRUE(f.IsReady());
    auto result = std::move(f).GetReadyResult();

    ASSERT_TRUE(result.HasError());
    ASSERT_THROW(result.ThrowIfError(), std::runtime_error);
  }

  SIMPLE_TEST(Subscribe1) {
    auto [f, p] = futures::MakeContract<int>();

    bool done = false;

    std::move(f).Subscribe([&done](wheels::Result<int> result) {
      ASSERT_EQ(result.ExpectValue(), 7);
      done = true;
    });

    ASSERT_FALSE(done);

    // Run callback here
    std::move(p).SetValue(7);

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(Subscribe2) {
    auto [f, p] = futures::MakeContract<int>();

    std::move(p).SetValue(12);

    bool done = false;

    // Run callback immediately
    std::move(f).Subscribe([&done](wheels::Result<int> result) {
      ASSERT_EQ(result.ExpectValue(), 12);
      done = true;
    });

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(Subscribe3) {
    executors::ThreadPool pool{4};

    auto [f, p] = futures::MakeContract<int>();

    executors::Execute(pool, [p = std::move(p)]() mutable {
      std::this_thread::sleep_for(1s);
      std::move(p).SetValue(7);
    });

    bool done = false;

    std::move(f).Subscribe([&done](wheels::Result<int> result) {
      ASSERT_EQ(result.ExpectValue(), 7);
      done = true;
    });

    pool.WaitIdle();

    ASSERT_TRUE(done);

    pool.Stop();
  }

  SIMPLE_TEST(SubscribeVia) {
    executors::ManualExecutor manual;

    auto [f, p] = futures::MakeContract<int>();

    std::move(p).SetValue(11);

    bool done = false;

    std::move(f).Via(manual).Subscribe([&done](wheels::Result<int> result) {
      ASSERT_EQ(result.ExpectValue(), 11);
      done = true;
    });

    ASSERT_FALSE(done);

    ASSERT_EQ(manual.Drain(), 1);

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(BlockingGet1) {
    auto [f, p] = futures::MakeContract<int>();

    std::move(p).SetValue(3);

    auto result = futures::GetResult(std::move(f));
    ASSERT_EQ(result.ExpectValue(), 3);
  }

  SIMPLE_TEST(BlockingGet2) {
    auto [f, p] = futures::MakeContract<int>();

    executors::ThreadPool pool{4};

    executors::Execute(pool, [p = std::move(p)]() mutable {
      std::this_thread::sleep_for(2s);
      std::move(p).SetValue(17);
    });

    {
      wheels::test::CpuTimeBudgetGuard cpu_time_budget{100ms};

      auto result = futures::GetResult(std::move(f));
      ASSERT_EQ(result.ExpectValue(), 17);
    }

    pool.Stop();
  }

  SIMPLE_TEST(Then1) {
    auto [f, p] = futures::MakeContract<int>();

    auto g = std::move(f).Then([](int value) {
      return value + 1;
    });

    std::move(p).SetValue(3);
    ASSERT_EQ(std::move(g).GetReadyResult().ExpectValue(), 4);
  }

  SIMPLE_TEST(Then2) {
    wheels::test::CpuTimeBudgetGuard cpu_time_budget{100ms};

    executors::ThreadPool pool{4};

    auto f1 = futures::Execute(pool, []() {
      std::this_thread::sleep_for(1s);
      return 42;
    });

    auto f2 = std::move(f1).Then([](int value) {
      return value + 1;
    });

    ASSERT_EQ(futures::GetResult(std::move(f2)), 43);

    pool.Stop();
  }

  SIMPLE_TEST(Then3) {
    wheels::test::CpuTimeBudgetGuard cpu_time_budget{100ms};

    executors::ThreadPool pool{4};

    auto [f, p] = futures::MakeContract<int>();

    std::move(p).SetValue(11);

    auto f2 = std::move(f).Via(pool).Then([&](int value) {
      ASSERT_TRUE(executors::ThreadPool::Current() == &pool);
      return value + 1;
    });

    int value = futures::GetResult(std::move(f2)).ExpectValue();

    ASSERT_EQ(value, 12);

    pool.Stop();
  }

  SIMPLE_TEST(Then4) {
    auto [f, p] = futures::MakeContract<int>();

    auto g = std::move(f).Then([](int /*value*/) -> Unit {
      FAIL_TEST("Skip continuation if error");
      return {};
    });

    std::move(p).SetError(TimeoutError());
    ASSERT_TRUE(std::move(g).GetReadyResult().HasError());
  }

  // Then chaining

  SIMPLE_TEST(Pipeline) {
    executors::ManualExecutor manual;

    bool done = false;

    futures::Execute(manual,
                     []() {
                       return 1;
                     })
        .Then([](int value) {
          return value + 1;
        })
        .Then([](int value) {
          return value + 2;
        })
        .Then([](int value) {
          return value + 3;
        })
        .Subscribe([&done](wheels::Result<int> result) {
          std::cout << "Value = " << result.ExpectValue() << std::endl;
          done = true;
        });

    size_t tasks = manual.Drain();
    ASSERT_EQ(tasks, 5);

    ASSERT_TRUE(done);
  }

  // Then chaining

  SIMPLE_TEST(PipelineError) {
    executors::ManualExecutor manual;

    bool done = false;

    futures::Execute(manual,
                     []() {
                       return 1;
                     })
        .Then([](int) -> int {
          throw std::runtime_error("Fail");
        })
        .Then([](int) -> int {
          std::abort();  // Skipped
        })
        .Then([](int) -> int {
          std::abort();  // Skipped
        })
        .Subscribe([&done](wheels::Result<int> result) {
          ASSERT_TRUE(result.HasError());
          done = true;
        });

    size_t tasks = manual.Drain();
    ASSERT_EQ(tasks, 5);

    ASSERT_TRUE(done);
  }

  // Then + Recover

  SIMPLE_TEST(PipelineRecover) {
    wheels::test::CpuTimeBudgetGuard cpu_time_budget{100ms};

    executors::ManualExecutor manual;

    bool done = false;

    futures::Execute(manual,
                     []() {
                       return 1;
                     })
        .Then([](int) -> int {
          throw std::runtime_error("Fail");
        })
        .Then([](int) -> int {
          std::abort();  // Skipped
        })
        .Recover([](wheels::Error) {
          return wheels::make_result::Ok(7);
        })
        .Then([](int value) {
          return value + 1;
        })
        .Subscribe([&done](wheels::Result<int> result) {
          ASSERT_EQ(result.ExpectValue(), 8);
          done = true;
        });

    size_t tasks = manual.Drain();
    ASSERT_EQ(tasks, 6);

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(PipelineRecoverWithoutError) {
    wheels::test::CpuTimeBudgetGuard cpu_time_budget{100ms};

    executors::ManualExecutor manual;

    bool done = false;

    futures::Execute(manual,
                     []() {
                       return 1;
                     })
        .Then([](int result) -> int {
          return result * 2;
        })
        .Then([](int result) -> int {
          return result * 10;
        })
        .Recover([](wheels::Error) {
          return wheels::make_result::Ok(7);
        })
        .Then([](int value) {
          return value + 1;
        })
        .Subscribe([&done](wheels::Result<int> result) {
          ASSERT_EQ(result.ExpectValue(), 21);
          done = true;
        });

    size_t tasks = manual.Drain();
    ASSERT_EQ(tasks, 6);

    ASSERT_TRUE(done);
  }

  // Via + Then

  SIMPLE_TEST(Via) {
    executors::ManualExecutor manual1;
    executors::ManualExecutor manual2;

    auto [f, p] = futures::MakeContract<int>();

    int step = 0;

    auto f2 = std::move(f)
                  .Via(manual1)
                  .Then([&](int value) {
                    step = 1;
                    return value + 1;
                  })
                  .Then([&](int value) {
                    step = 2;
                    return value + 2;
                  })
                  .Via(manual2)
                  .Then([&](int value) {
                    step = 3;
                    return value + 3;
                  });

    // Launch pipeline
    std::move(p).SetValue(0);

    ASSERT_EQ(manual1.Drain(), 2);
    ASSERT_EQ(step, 2);
    ASSERT_EQ(manual2.Drain(), 1);
    ASSERT_EQ(step, 3);

    auto f3 = std::move(f2)
                  .Then([&](int value) {
                    step = 4;
                    return value + 4;
                  })
                  .Via(manual1)
                  .Then([&](int value) {
                    step = 5;
                    return value + 5;
                  });

    ASSERT_EQ(manual2.Drain(), 1);
    ASSERT_EQ(step, 4);

    ASSERT_EQ(manual1.Drain(), 1);
    ASSERT_EQ(step, 5);

    ASSERT_TRUE(f3.IsReady());
    auto value = std::move(f3).GetReadyResult().ExpectValue();
    ASSERT_EQ(value, 1 + 2 + 3 + 4 + 5);
  }

  // Asynchronous Then

  SIMPLE_TEST(AsyncThen) {
    wheels::test::CpuTimeBudgetGuard cpu_time_budget{100ms};

    executors::ThreadPool pool1{4};
    executors::ThreadPool pool2{4};

    auto pipeline = futures::Execute(pool1,
                                     []() -> int {
                                       return 1;
                                     })
                        .Then([&pool2](int value) {
                          return futures::Execute(pool2, [value]() {
                            return value + 1;
                          });
                        })
                        .Then([&pool1](int value) {
                          return futures::Execute(pool1, [value]() {
                            return value + 2;
                          });
                        });

    int value = futures::GetValue(std::move(pipeline));

    pool1.Stop();
    pool2.Stop();

    ASSERT_EQ(value, 4);
  }

  SIMPLE_TEST(All1) {
    auto [f1, p1] = futures::MakeContract<int>();
    auto [f2, p2] = futures::MakeContract<int>();
    auto [f3, p3] = futures::MakeContract<int>();

    auto all = futures::All(std::move(f1), std::move(f2), std::move(f3));

    ASSERT_FALSE(all.IsReady());

    std::move(p3).SetValue(7);
    std::move(p1).SetValue(3);

    // Still not completed
    ASSERT_FALSE(all.IsReady());

    std::move(p2).SetValue(5);

    ASSERT_TRUE(all.IsReady());

    auto all_result = std::move(all).GetReadyResult();
    ASSERT_TRUE(all_result.IsOk());

    ASSERT_EQ(*all_result, std::vector<int>({3, 5, 7}));
  }

  SIMPLE_TEST(All2) {
    auto [f1, p1] = futures::MakeContract<int>();
    auto [f2, p2] = futures::MakeContract<int>();

    auto all = futures::All(std::move(f1), std::move(f2));

    ASSERT_FALSE(all.IsReady());

    // First error
    std::move(p2).SetError(TimeoutError());

    ASSERT_TRUE(all.IsReady());

    // Second error
    std::move(p1).SetError(TimeoutError());

    ASSERT_TRUE(all.IsReady());

    auto all_result = std::move(all).GetReadyResult();
    ASSERT_TRUE(all_result.HasError());
  }

  SIMPLE_TEST(AllNonDefaultConstructable) {
    auto [f1, p1] = futures::MakeContract<NonDefaultConstructable>();
    auto [f2, p2] = futures::MakeContract<NonDefaultConstructable>();

    auto all = futures::All(std::move(f1), std::move(f2));

    ASSERT_FALSE(all.IsReady());

    std::move(p1).SetValue(NonDefaultConstructable{1});

    ASSERT_FALSE(all.IsReady());

    std::move(p2).SetValue(NonDefaultConstructable{2});

    ASSERT_TRUE(all.IsReady());

    auto all_result = std::move(all).GetReadyResult();
    ASSERT_TRUE(all_result.IsOk());

    auto all_values = std::move(*all_result);

    ASSERT_EQ(all_values[0].value, 1);
    ASSERT_EQ(all_values[1].value, 2);
  }

  SIMPLE_TEST(AllEmptyInput) {
    auto all = All(std::vector<futures::Future<int>>{});

    ASSERT_TRUE(all.IsReady());
    ASSERT_TRUE(std::move(all).GetReadyResult().IsOk());
  }

  SIMPLE_TEST(FirstOf1) {
    auto [f1, p1] = futures::MakeContract<int>();
    auto [f2, p2] = futures::MakeContract<int>();

    auto first_of = futures::FirstOf(std::move(f1), std::move(f2));

    ASSERT_FALSE(first_of.IsReady());

    std::move(p2).SetValue(13);

    ASSERT_TRUE(first_of.IsReady());

    std::move(p1).SetError(TimeoutError());

    auto first_result = std::move(first_of).GetReadyResult();

    ASSERT_EQ(first_result.ExpectValue(), 13);
  }

  SIMPLE_TEST(FirstOf2) {
    auto [f1, p1] = futures::MakeContract<int>();
    auto [f2, p2] = futures::MakeContract<int>();
    auto [f3, p3] = futures::MakeContract<int>();

    auto first_of = futures::FirstOf(std::move(f1), std::move(f2), std::move(f3));

    ASSERT_FALSE(first_of.IsReady());

    std::move(p1).SetError(TimeoutError());

    ASSERT_FALSE(first_of.IsReady());

    std::move(p3).SetError(TimeoutError());

    ASSERT_FALSE(first_of.IsReady());

    std::move(p2).SetValue(44);

    ASSERT_TRUE(first_of.IsReady());

    auto first_result = std::move(first_of).GetReadyResult();

    ASSERT_EQ(first_result.ExpectValue(), 44);
  }

  SIMPLE_TEST(FirstOfNonDefaultConstructable) {
    auto [f1, p1] = futures::MakeContract<NonDefaultConstructable>();
    auto [f2, p2] = futures::MakeContract<NonDefaultConstructable>();

    auto first_of = futures::FirstOf(std::move(f1), std::move(f2));

    ASSERT_FALSE(first_of.IsReady());

    std::move(p2).SetValue(NonDefaultConstructable{2});

    ASSERT_TRUE(first_of.IsReady());

    std::move(p1).SetValue(NonDefaultConstructable{1});

    ASSERT_TRUE(first_of.IsReady());

    auto first_result = std::move(first_of).GetReadyResult();
    ASSERT_TRUE(first_result.IsOk());
    ASSERT_EQ(first_result->value, 2);
  }

  SIMPLE_TEST(Combine) {
    auto [f1, p1] = futures::MakeContract<int>();
    auto [f2, p2] = futures::MakeContract<int>();
    auto [f3, p3] = futures::MakeContract<int>();

    auto first_of_12 = futures::FirstOf(std::move(f1), std::move(f2));
    auto f = futures::All(std::move(first_of_12), std::move(f3));

    std::move(p2).SetError(TimeoutError());
    std::move(p3).SetValue(3);

    ASSERT_FALSE(f.IsReady());

    std::move(p1).SetValue(1);

    ASSERT_TRUE(f.IsReady());
  }
}

RUN_ALL_TESTS()
