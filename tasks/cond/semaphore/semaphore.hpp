#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

class Semaphore {
 public:
  explicit Semaphore(size_t /*tokens*/) {
    // Not implemented
  }

  void Acquire() {
    // Not implemented
  }

  void Release() {
    // Not implemented
  }

 private:
  // Tokens
};
