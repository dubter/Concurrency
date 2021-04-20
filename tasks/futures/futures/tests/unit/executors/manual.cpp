#include <await/executors/static_thread_pool.hpp>
#include <await/executors/priority.hpp>
#include <await/executors/manual.hpp>
#include <await/executors/label_thread.hpp>

#include <wheels/test/test_framework.hpp>

#include "../helpers.hpp"

#include <thread>
#include <atomic>

using namespace await::executors;
using namespace std::chrono_literals;

TEST_SUITE(Manual) {
  class Looper {
   public:
    Looper(std::function<void()> task, std::weak_ptr<IExecutor> e)
        : task_(std::move(task)), e_(std::move(e)) {
    }

    void operator()() {
      task_();
      if (auto e = e_.lock()) {
        e->Execute(Looper(task_, e_));
      }
    }

   private:
    std::function<void()> task_;
    std::weak_ptr<IExecutor> e_;
  };

  SIMPLE_TEST(RunAtMost) {
    auto manual = MakeManualExecutor();

    int steps = 0;

    manual->Execute(Looper([&steps]() {
      ++steps;
    }, manual));

    manual->RunAtMost(3);

    ASSERT_EQ(steps, 3);

    manual->Clear();
  }

  SIMPLE_TEST(RunAtMostOne) {
    auto manual = MakeManualExecutor();

    int steps = 0;

    ASSERT_FALSE(manual->RunAtMostOne());

    manual->Execute(Looper([&steps]() {
      ++steps;
    }, manual));

    ASSERT_TRUE(manual->RunAtMostOne());
    ASSERT_EQ(steps, 1);

    ASSERT_TRUE(manual->RunAtMostOne());
    ASSERT_EQ(steps, 2);

    manual->Clear();
  }

  SIMPLE_TEST(RunAllScheduledTasks) {
    auto manual = MakeManualExecutor();

    manual->Execute([]() {});
    manual->Execute([]() {});
    manual->Execute(Looper([]() {}, manual));

    ASSERT_EQ(manual->RunAllScheduled(), 3);
    ASSERT_EQ(manual->RunAllScheduled(), 1);
    ASSERT_EQ(manual->RunAllScheduled(), 1);

    manual->Clear();
  }

  SIMPLE_TEST(Drain) {
    auto manual = MakeManualExecutor();

    bool done = false;

    manual->Execute([&done, manual]() {
      manual->Execute([&done]() {
        done = true;
      });
    });

    manual->Drain();

    ASSERT_TRUE(done);

    manual->Clear();
  }
}
