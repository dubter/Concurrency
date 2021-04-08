#pragma once

#include <wheels/test/test_framework.hpp>

#include <wheels/support/time.hpp>
#include <wheels/support/cpu_time.hpp>
#include <wheels/support/error.hpp>

#include <chrono>
#include <ctime>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace test_helpers {

////////////////////////////////////////////////////////////////////////////////

class WallTimeLimitGuard {
  using Duration = wheels::Duration;
 public:
  WallTimeLimitGuard(Duration limit) : limit_(limit) {
  }

  ~WallTimeLimitGuard() {
    ASSERT_TRUE(stop_watch_.Elapsed() <= limit_);
  }

 private:
  wheels::StopWatch<> stop_watch_;
  Duration limit_;
};


////////////////////////////////////////////////////////////////////////////////

#if __has_feature(thread_sanitizer) || __has_feature(address_sanitizer)

class CPUTimeBudgetGuard {
 public:
  CPUTimeBudgetGuard(wheels::Duration) {
  }
};

#else

class CPUTimeBudgetGuard {
 public:
  CPUTimeBudgetGuard(wheels::Duration limit) : limit_(limit) {
  }

  ~CPUTimeBudgetGuard() {
    auto usage = meter_.Elapsed();
    std::cout << "CPU usage: " << ToSeconds(usage) << " seconds" << std::endl;
    ASSERT_TRUE(usage <= limit_);
  }

 private:
  static double ToSeconds(wheels::Duration d) {
    return 1.0 * d.count() / 1'000'000'000;
  }

 private:
  wheels::ProcessCPUTimer meter_;
  wheels::Duration limit_;
};

#endif

////////////////////////////////////////////////////////////////////////////////

class OnePassBarrier {
 public:
  OnePassBarrier(size_t threads) : total_(threads) {
  }

  void Pass() {
    arrived_.fetch_add(1);
    while (arrived_.load() < total_) {
      std::this_thread::yield();
    }
  }

 private:
  size_t total_{0};
  std::atomic<size_t> arrived_{0};
};

////////////////////////////////////////////////////////////////////////////////

class OneShotEvent {
 public:
  void Await() {
    std::unique_lock lock(mutex_);
    while (!set_) {
      set_cond_.wait(lock);
    }
  }

  void Set() {
    std::lock_guard guard(mutex_);
    set_ = true;
    set_cond_.notify_one();
  }

 private:
  bool set_{false};
  std::mutex mutex_;
  std::condition_variable set_cond_;
};

////////////////////////////////////////////////////////////////////////////////

inline wheels::Error MakeTestError() {
  return {std::make_error_code(std::errc::timed_out)};
}

}  // namespace test_helpers