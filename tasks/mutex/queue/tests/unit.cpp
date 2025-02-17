#include "../queue_spinlock.hpp"

#include <wheels/test/framework.hpp>

#include <thread>
#include <iostream>

TEST_SUITE(QueueSpinLock) {
  SIMPLE_TEST(LockUnlock) {
    QueueSpinLock spinlock;

    {
      QueueSpinLock::Guard guard(spinlock);  // <-- Acquired
      // Critical section
    }  // <-- Released
  }

  SIMPLE_TEST(SequentialLockUnlock) {
    QueueSpinLock spinlock;

    {
      QueueSpinLock::Guard guard(spinlock);
      // Critical section
    }

    {
      QueueSpinLock::Guard guard(spinlock);
      // Critical section
    }
  }

  SIMPLE_TEST(ConcurrentIncrements) {
    QueueSpinLock spinlock;
    size_t shared_counter = 0;

    const size_t kIncrementsPerThread = 1000;

    auto contender = [&]() {
      for (size_t i = 0; i < kIncrementsPerThread; ++i) {
        QueueSpinLock::Guard guard(spinlock);

        size_t current = shared_counter;
        std::this_thread::yield();
        shared_counter = current + 1;
      }
    };

    std::thread t1(contender);
    std::thread t2(contender);
    t1.join();
    t2.join();

    std::cout << "Shared counter value: " << shared_counter << std::endl;

    ASSERT_EQ(shared_counter, 2 * kIncrementsPerThread);
  }
}

RUN_ALL_TESTS()
