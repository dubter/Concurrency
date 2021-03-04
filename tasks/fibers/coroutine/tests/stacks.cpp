#include <mtf/fibers/api.hpp>

#include <wheels/test/test_framework.hpp>

using mtf::fibers::Spawn;

TEST_SUITE(Stacks) {

#if !__has_feature(thread_sanitizer)

  TEST(Pool, wheels::test::TestOptions().TimeLimit(5s).AdaptTLToSanitizer()) {
    mtf::tp::StaticThreadPool scheduler{1};

    static const size_t kFibers = 1'000'000;

    std::atomic<size_t> counter{0};

    for (size_t i = 0; i < kFibers; ++i) {
      Spawn([&]() {
        ++counter;
      }, scheduler);
    }

    scheduler.Join();

    ASSERT_EQ(counter, kFibers)
  }

#endif

}
