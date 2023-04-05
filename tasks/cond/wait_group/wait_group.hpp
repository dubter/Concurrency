#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <twist/ed/wait/sys.hpp>
#include <cstdlib>

class WaitGroup {
 public:
  // += count
  void Add(size_t count) {
    std::lock_guard lock(mutex_);
    counter_ += count;
  }

  // =- 1
  void Done() {
    std::lock_guard lock(mutex_);
    counter_--;
    if (counter_ == 0) {
      waiting_.notify_all();
    }
  }

  // == 0
  // One-shot
  void Wait() {
    std::unique_lock lock(mutex_);
    while (counter_ != 0) {
      waiting_.wait(lock);
    }
  }

 private:
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable waiting_;
  uint32_t counter_{0};
};
