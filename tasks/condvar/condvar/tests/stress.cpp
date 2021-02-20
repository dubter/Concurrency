#include "../condvar.hpp"

#include <twist/test/test.hpp>
#include <twist/test/runs.hpp>
#include <twist/test/random.hpp>

#include <twist/test/util/race.hpp>

#include <twist/stdlike/mutex.hpp>

#include <wheels/test/util.hpp>

#include <atomic>
#include <deque>

////////////////////////////////////////////////////////////////////////////////

namespace robot {

void NotifyAtLeastOneThread(solutions::ConditionVariable& condvar) {
  if (twist::test::Random2()) {
    condvar.NotifyOne();
  } else {
    condvar.NotifyAll();
  }
}

class Robot {
  enum class Step { Left, Right };

 public:
  void LeftStep() {
    std::unique_lock lock(mutex_);
    while (step_ != Step::Left) {
      switched_.Wait(lock);
    }
    ASSERT_TRUE(step_ == Step::Left);
    step_ = Step::Right;
    lock.unlock();
    NotifyAtLeastOneThread(switched_);
  }

  void RightStep() {
    std::unique_lock lock(mutex_);
    while (step_ != Step::Right) {
      switched_.Wait(lock);
    }
    ASSERT_TRUE(step_ == Step::Right);
    step_ = Step::Left;
    NotifyAtLeastOneThread(switched_);
  }

 private:
  Step step_{Step::Left};
  twist::stdlike::mutex mutex_;
  solutions::ConditionVariable switched_;
};

void Test(size_t steps) {
  Robot robot;

  twist::test::util::Race race{2};

  race.Add([&]() {
    for (size_t i = 0; i < steps; ++i) {
      robot.LeftStep();
    }
  });
  race.Add([&]() {
    for (size_t j = 0; j < steps; ++j) {
      robot.RightStep();
    }
  });

  race.Run();
}
}  // namespace robot

TWIST_TEST_RUNS(Robot, robot::Test)
    ->TimeLimit(10s)
    ->Run(10'000)
    ->TimeLimit(30s)
    ->Run(100'000);

#if defined(TWIST_FIBERS)

TWIST_TEST_RUNS(RobotExt, robot::Test)->TimeLimit(10s)->Run(1000'000);

#endif

////////////////////////////////////////////////////////////////////////////////

namespace queue {

template <typename T>
class UnboundedBlockingQueue {
 public:
  void Put(T value) {
    std::unique_lock lock(mutex_);
    buffer_.push_back(std::move(value));

    if (twist::test::Random2()) {
      lock.unlock();
    }
    not_empty_.NotifyOne();
  }

  T Take() {
    std::unique_lock lock(mutex_);
    while (buffer_.empty()) {
      not_empty_.Wait(lock);
    }
    return TakeLocked();
  }

 private:
  T TakeLocked() {
    T front = std::move(buffer_.front());
    buffer_.pop_front();
    return front;
  }

 private:
  std::deque<int> buffer_;
  twist::stdlike::mutex mutex_;
  solutions::ConditionVariable not_empty_;
};

void Test(size_t producers, size_t consumers) {
  UnboundedBlockingQueue<int> queue_;

  std::atomic<int> consumed{0};
  std::atomic<int> produced{0};

  std::atomic<size_t> producers_left{producers};

  twist::test::util::Race race{producers + consumers};

  // Producers

  for (size_t p = 0; p < producers; ++p) {
    race.Add([&, p]() {
      int value = p;
      while (wheels::test::KeepRunning()) {
        queue_.Put(value);
        produced.fetch_add(value);
        value += producers;
      }

      if (producers_left.fetch_sub(1) == 1) {
        // Last producer
        for (size_t j = 0; j < consumers; ++j) {
          // Put poison pill
          queue_.Put(-1);
        }
      }
    });
  }

  // Consumers

  for (size_t j = 0; j < consumers; ++j) {
    race.Add([&]() {
      while (true) {
        int value = queue_.Take();
        if (value == -1) {
          break;  // Poison pill
        }
        consumed.fetch_add(value);
      }
    });
  }

  race.Run();

  ASSERT_EQ(consumed.load(), produced.load());
}
}  // namespace queue

TWIST_TEST_RUNS(ProducersConsumers, queue::Test)
    ->TimeLimit(10s)
    ->Run(5, 1)
    ->Run(1, 5)
    ->TimeLimit(20s)
    ->Run(5, 5)
    ->Run(10, 10);

////////////////////////////////////////////////////////////////////////////////

RUN_ALL_TESTS()
