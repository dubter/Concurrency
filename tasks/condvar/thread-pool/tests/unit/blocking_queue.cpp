#include <tp/blocking_queue.hpp>

#include <wheels/test/test_framework.hpp>

#include <wheels/support/cpu_time.hpp>

#include <string>
#include <thread>

using wheels::ProcessCPUTimer;
using wheels::ThreadCPUTimer;

template <typename T>
using Queue = tp::UnboundedBlockingQueue<T>;

TEST_SUITE(BlockingQueue) {
  SIMPLE_TEST(JustWorks) {
    Queue<int> q;

    q.Put(7);
    auto value = q.Take();
    ASSERT_TRUE(value);
    ASSERT_EQ(*value, 7);

    q.Close();
    ASSERT_FALSE(q.Take());
  }

  SIMPLE_TEST(Fifo) {
    Queue<int> q;
    q.Put(1);
    q.Put(2);
    q.Put(3);

    ASSERT_EQ(*q.Take(), 1);
    ASSERT_EQ(*q.Take(), 2);
    ASSERT_EQ(*q.Take(), 3);
  }

  SIMPLE_TEST(Close) {
    Queue<std::string> q;

    q.Put("Hello");
    q.Put(",");
    q.Put("World");

    q.Close();

    ASSERT_FALSE(q.Put("!"));

    ASSERT_EQ(*q.Take(), "Hello");
    ASSERT_EQ(*q.Take(), ",");
    ASSERT_EQ(*q.Take(), "World");
    ASSERT_FALSE(q.Take());
  }

  SIMPLE_TEST(Cancel) {
    Queue<std::string> q;

    q.Put("Hello");
    q.Put(",");
    q.Put("World");

    q.Close();

    ASSERT_FALSE(q.Put("!"));

    ASSERT_EQ(*q.Take(), "Hello");
    ASSERT_EQ(*q.Take(), ",");
    ASSERT_EQ(*q.Take(), "World");
    ASSERT_FALSE(q.Take());
  }

  struct MoveOnly {
    MoveOnly() = default;

    MoveOnly(const MoveOnly& that) = delete;
    MoveOnly& operator=(const MoveOnly& that) = delete;

    MoveOnly(MoveOnly&& that) = default;
    MoveOnly& operator=(MoveOnly&& that) = default;
  };

  SIMPLE_TEST(MoveOnly) {
    Queue<MoveOnly> queue;

    queue.Put(MoveOnly{});
    ASSERT_TRUE(queue.Take().has_value());
  }

  SIMPLE_TEST(BlockingTake) {
    Queue<int> q;

    std::thread producer([&]() {
      std::this_thread::sleep_for(1s);
      q.Put(7);
    });

    ThreadCPUTimer thread_cpu_timer;

    auto value = q.Take();

    auto elapsed = thread_cpu_timer.Elapsed();

    ASSERT_TRUE(value);
    ASSERT_EQ(*value, 7);
    ASSERT_TRUE(elapsed < 100ms);

    producer.join();
  }

  SIMPLE_TEST(ProducerConsumer) {
    Queue<int> q;

    ProcessCPUTimer process_cpu_timer;

    std::thread producer([&]() {
      // Producer
      for (int i = 0; i < 10; ++i) {
        q.Put(i);
        std::this_thread::sleep_for(100ms);
      }
      q.Close();
    });

    // Consumer

    for (int i = 0; i < 10; ++i) {
      auto value = q.Take();
      ASSERT_TRUE(value);
      ASSERT_EQ(*value, i);
    }

    ASSERT_FALSE(q.Take());

    producer.join();

    ASSERT_TRUE(process_cpu_timer.Elapsed() < 100ms);
  }
}
