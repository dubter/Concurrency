#include "guarded.hpp"

#include <twist/test/test.hpp>

#include <twist/stdlike/thread.hpp>

#include <twist/strand/stdlike.hpp>

#include <chrono>
#include <set>
#include <string>
#include <vector>

using namespace std::chrono_literals;

using solutions::Guarded;

TEST_SUITE(Guarded) {
  SIMPLE_TWIST_TEST(Vector) {
    Guarded<std::vector<int>> ints;

    ASSERT_TRUE(ints->empty());

    ints->push_back(42);
    ASSERT_EQ(ints->front(), 42);
    ASSERT_EQ(ints->at(0), 42);
    ASSERT_EQ(ints->size(), 1);

    ints->push_back(99);
    ASSERT_EQ(ints->size(), 2);
  }

  SIMPLE_TWIST_TEST(Set) {
    Guarded<std::set<std::string>> strings;

    strings->insert("Hello");
    strings->insert("World");
    strings->insert("!");

    ASSERT_EQ(strings->size(), 3);
  }

  SIMPLE_TWIST_TEST(Ctor) {
    Guarded<std::string> str(5, '!');
    ASSERT_EQ(str->length(), 5);
  }

  class Counter {
   public:
    void Increment() {
      size_t value = value_;
      twist::strand::stdlike::this_thread::sleep_for(1s);
      value_ = value + 1;
    }

    size_t Value() const {
      return value_;
    }

   private:
    size_t value_{0};
  };

  SIMPLE_TWIST_TEST(Counter) {
    Guarded<Counter> counter;

    twist::stdlike::thread t1([&]() {
      counter->Increment();
    });
    twist::stdlike::thread t2([&]() {
      counter->Increment();
    });

    t1.join();
    t2.join();

    ASSERT_EQ(counter->Value(), 2);
  }
}

RUN_ALL_TESTS()
