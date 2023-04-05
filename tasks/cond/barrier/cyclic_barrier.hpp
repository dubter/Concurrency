#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <cstdlib>

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t num_threads)
      : num_threads_(num_threads),
        barrier_state_(0),
        num_threads_arrived_{0, 0} {
  }

  void ArriveAndWait() {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);
    size_t current_barrier = barrier_state_;
    num_threads_arrived_[current_barrier]++;
    while (num_threads_arrived_[current_barrier] != num_threads_) {
      all_threads_arrived_.wait(lock);
    }
    if (barrier_state_ == current_barrier) {
      barrier_state_ = 1 - barrier_state_;
      num_threads_arrived_[barrier_state_] = 0;
      all_threads_arrived_.notify_all();
    }
  }

 private:
  size_t num_threads_;
  size_t barrier_state_;
  size_t num_threads_arrived_[2];
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable all_threads_arrived_;
};
