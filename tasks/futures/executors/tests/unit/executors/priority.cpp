#include <await/executors/static_thread_pool.hpp>
#include <await/executors/priority.hpp>
#include <await/executors/manual.hpp>
#include <await/executors/inline.hpp>
#include <await/executors/label_thread.hpp>

#include <wheels/test/test_framework.hpp>

#include "../helpers.hpp"

#include <thread>
#include <atomic>

using namespace await::executors;
using namespace std::chrono_literals;

TEST_SUITE(Priority) {
  SIMPLE_TEST(JustWorks) {
    auto manual = MakeManualExecutor();

    std::vector<int> ints;

    auto pq = MakePriorityExecutor(manual);

    pq->Execute(1, [&]() {
      ints.push_back(1);
    });
    pq->Execute(2, [&]() {
      ints.push_back(2);
    });
    pq->Execute(3, [&]() {
      ints.push_back(3);
    });

    manual->Drain();
    ASSERT_EQ(ints, std::vector<int>({3, 2, 1}));
  }

  SIMPLE_TEST(FixedPriority) {
    int value = 0;
    auto manual = MakeManualExecutor();

    auto pq = MakePriorityExecutor(manual);
    auto high = pq->FixPriority(1);
    auto low = pq->FixPriority(-1);

    low->Execute([&]() {
      value = -1;
    });
    high->Execute([&]() {
      value = 1;
    });

    ASSERT_EQ(value, 0);

    ASSERT_TRUE(manual->RunAtMostOne());
    ASSERT_EQ(value, 1);

    ASSERT_TRUE(manual->RunAtMostOne());
    ASSERT_EQ(value, -1);
  }

  SIMPLE_TEST(Lifetime1) {
    auto manual = MakeManualExecutor();

    bool done = false;

    MakePriorityExecutor(manual)->Execute(1, [&]() {
      done = true;
    });

    manual->Drain();
    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(Lifetime2) {
    auto manual = MakeManualExecutor();

    bool done = false;

    MakePriorityExecutor(manual)->FixPriority(1)->Execute([&]() {
      done = true;
    });

    manual->Drain();
    ASSERT_TRUE(done);
  }

  Task MakeLoop(std::weak_ptr<IPriorityExecutor> pq, int priority) {
    return [pq, priority]() {
      std::this_thread::sleep_for(100ms);
      if (auto e = pq.lock()) {
        e->Execute(priority, MakeLoop(pq, priority));
      }
    };
  }

  SIMPLE_TEST(LowPriorityTask) {
    auto tp = MakeStaticThreadPool(1, "test");
    auto pq = MakePriorityExecutor(tp);

    pq->Execute(100, MakeLoop(pq, 100));
    pq->Execute(-100, []() {
      std::abort();
    });

    std::this_thread::sleep_for(3s);

    tp->Shutdown();
  }

  SIMPLE_TEST(UseThreadPool) {
    auto tp = MakeStaticThreadPool(4, "tp");

    auto pq = MakePriorityExecutor(tp);

    pq->Execute(1, []() {
      ExpectThread("tp");
    });

    tp->Join();
  }

  TEST(Concurrent, wheels::test::TestOptions().AdaptTLToSanitizer()) {
    auto tasks_tp = MakeStaticThreadPool(4, "tasks");
    auto pq = MakePriorityExecutor(tasks_tp);

    auto clients_tp = MakeStaticThreadPool(4, "clients");

    static const size_t kTasks = 100500;

    for (size_t i = 0; i < kTasks; ++i) {
      clients_tp->Execute([pq]() {
        pq->Execute(33, []() {});
      });
    }

    clients_tp->Join();
    tasks_tp->Join();

    ASSERT_EQ(tasks_tp->ExecutedTaskCount(), kTasks);
  }

  SIMPLE_TEST(Mutex) {
    auto inlined = GetInlineExecutor();
    auto pq = MakePriorityExecutor(inlined);

    auto submit_sleep = [pq]() {
      pq->Execute(1, []() {
        std::this_thread::sleep_for(1s);
      });
    };

    wheels::StopWatch stop_watch;

    std::thread t1(submit_sleep);
    std::thread t2(submit_sleep);

    t1.join();
    t2.join();

    ASSERT_TRUE(stop_watch.Elapsed() < 1500ms);
  }
}
