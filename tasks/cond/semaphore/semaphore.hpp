#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <cstdlib>

class Semaphore {
 public:
  explicit Semaphore(size_t num_tokens)
      : num_tokens_(num_tokens) {
  }

  void Acquire() {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);
    while (num_tokens_ == 0) {
      has_tokens_.wait(lock);
    }
    num_tokens_--;
  }

  void Release() {
    std::lock_guard<twist::ed::stdlike::mutex> lock(mutex_);
    num_tokens_++;
    has_tokens_.notify_one();
  }

 private:
  size_t num_tokens_;
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable has_tokens_;
};
