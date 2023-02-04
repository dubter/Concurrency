#include "../../shared_ptr.hpp"

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/stdlike/thread.hpp>

#include <wheels/test/test_framework.hpp>
#include <twist/test/test.hpp>

void Race() {
  AtomicSharedPtr<std::string> asp;

  asp.Store(MakeShared<std::string>("Initial"));

  twist::ed::stdlike::thread reader([&]() {
    auto sp = asp.Load();
    ASSERT_TRUE(sp);
  });

  twist::ed::stdlike::thread writer([&]() {
    asp.Store(MakeShared<std::string>("Writer"));
  });

  reader.join();
  writer.join();
}

TEST_SUITE(AtomicSharedPtr) {
  TWIST_ITERATE_TEST(Race, 5s) {
    Race();
  }
}
