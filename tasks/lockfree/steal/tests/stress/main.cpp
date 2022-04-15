#include "../../work_stealing_queue.hpp"

#include <twist/fault/adversary/lockfree.hpp>

#include <twist/test/test.hpp>
#include <twist/test/util/race.hpp>
#include <twist/test/util/lockfree.hpp>

#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <array>
#include <iostream>

/////////////////////////////////////////////////////////////////////

struct TestObject {
  size_t value;

  static TestObject* New() {
    static std::atomic<size_t> next_{0};

    return new TestObject{next_.fetch_add(1)};
  }
};

/////////////////////////////////////////////////////////////////////

template <size_t Queues, size_t Capacity>
void StressTest() {
  twist::fault::SetAdversary(twist::fault::CreateLockFreeAdversary());

  std::array<twist::test::util::ReportProgressFor<lockfree::WorkStealingQueue<TestObject, Capacity>>, Queues> queues_;

  // Checksums
  std::atomic<size_t> produced_cs{0};
  std::atomic<size_t> consumed_cs{0};
  std::atomic<size_t> stolen_cs{0};

  twist::test::util::Race race;

  for (size_t i = 0; i < Queues; ++i) {
    race.Add([&, i]() {
      twist::fault::GetAdversary()->EnablePark();

      size_t random = i;  // Seed

      TestObject* obj_to_push = TestObject::New();

      for (size_t iter = 0; wheels::test::KeepRunning(); ++iter) {
        // TryPush

        size_t curr_value = obj_to_push->value;

        if (queues_[i]->TryPush(obj_to_push)) {
          obj_to_push = TestObject::New();
          random += curr_value;
          produced_cs.fetch_add(curr_value);
        }

        // Grab

        TestObject* steal_buffer[5];

        if ((iter + i) % 7 == 0) {
          size_t steal_target = random % Queues;  // Pseudo-random target

          size_t stolen = queues_[steal_target]->Grab({steal_buffer, 5});

          for (size_t s = 0; s < stolen; ++s) {
            stolen_cs.fetch_add(steal_buffer[s]->value);
            delete steal_buffer[s];
          }
          continue;
        }

        // TryPop

        if (random % 2 == 0) {
          continue;
        }

        if (TestObject* obj = queues_[i]->TryPop()) {
          consumed_cs.fetch_add(obj->value);
          delete obj;
        }
      }

      // Cleanup

      while (TestObject* obj = queues_[i]->TryPop()) {
        consumed_cs += obj->value;
        delete obj;
      }
    });
  }

  race.Run();

  std::cout << "Produced: " << produced_cs.load() << std::endl;
  std::cout << "Consumed: " << consumed_cs.load() << std::endl;
  std::cout << "Stolen: "   << stolen_cs.load() << std::endl;

  ASSERT_EQ(produced_cs.load(), consumed_cs.load() + stolen_cs.load());
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(WorkStealingQueue) {
  TWIST_TEST_TL(Stress_1, 5s) {
    StressTest</*Queues=*/2,  /*Capacity=*/5>();
  }

  TWIST_TEST_TL(Stress_2, 5s) {
    StressTest</*Queues=*/4, /*Capacity=*/16>();
  }

  TWIST_TEST_TL(Stress_3, 5s) {
    StressTest</*Queues=*/4, /*Capacity=*/33>();
  }

  TWIST_TEST_TL(Stress_4, 5s) {
    StressTest</*Queues=*/4, /*Capacity=*/128>();
  }
}

RUN_ALL_TESTS()
