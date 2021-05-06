#include <await/executors/static_thread_pool.hpp>
#include <await/executors/strand.hpp>

#include <twist/test/test.hpp>

#include <twist/stdlike/thread.hpp>

#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

using namespace await::executors;
using namespace std::chrono_literals;

/////////////////////////////////////////////////////////////////////

class Counter {
 public:
  Counter(IExecutorPtr e)
      : strand_(MakeStrand(e)) {
  }

  void AsyncIncrement() {
    strand_->Execute([this]() {
      ++value_;
    });
  }

  size_t Value() const {
    return value_;
  }

 private:
  size_t value_{0};
  IExecutorPtr strand_;
};

void ConcurrentStrands(size_t strands, size_t iterations, size_t batch_size) {
  auto tp = MakeStaticThreadPool(16, "pool");

  std::vector<Counter> counters;
  counters.reserve(strands);
  for (size_t i = 0; i < strands; ++i) {
    counters.emplace_back(tp);
  }

  for (size_t i = 0; i < iterations; ++i) {
    for (size_t j = 0; j < strands; ++j) {
      for (size_t k = 0; k < batch_size; ++k) {
        counters[j].AsyncIncrement();
      }
    }
  }

  tp->Join();

  for (size_t i = 0; i < strands; ++i) {
    ASSERT_EQ(counters[i].Value(), batch_size * iterations);
  }
}

//////////////////////////////////////////////////////////////////////

void MissingTasks() {
  auto tp = MakeStaticThreadPool(3, "test");

  size_t i = 0;

  while (wheels::test::KeepRunning()) {
    auto strand = MakeStrand(tp);

    std::atomic<size_t> completed_tasks{0};

    auto task = [&completed_tasks]() {
      completed_tasks.store(completed_tasks.load() + 1);
    };

    size_t tasks = 2 + (i++) % 5;

    for (size_t t = 0; t < tasks; ++t) {
      strand->Execute(task);
    }

    while (completed_tasks != tasks) {
      twist::stdlike::this_thread::yield();
    }
  }

  tp->Join();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Strand) {
  TWIST_TEST_TL(MissingTasks, 5s) {
    MissingTasks();
  }

  TWIST_TEST_TL(ConcurrentStrands1, 10s) {
    ConcurrentStrands(50, 50, 50);
  }

  TWIST_TEST_TL(ConcurrentStrands2, 10s) {
    ConcurrentStrands(100, 100, 20);
  }
}
