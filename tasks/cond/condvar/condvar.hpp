#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdint>

namespace stdlike {

class CondVar {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable
  template <class Mutex>
  void Wait(Mutex& mutex) {
    auto value = value_.load();
    mutex.unlock();
    twist::ed::Wait(value_, value);
    mutex.lock();
  }

  void NotifyOne() {
    auto wake_key = twist::ed::PrepareWake(value_);
    value_.fetch_add(1);
    twist::ed::WakeOne(wake_key);
  }

  void NotifyAll() {
    auto wake_key = twist::ed::PrepareWake(value_);
    value_.fetch_add(1);
    twist::ed::WakeAll(wake_key);
  }

 private:
  twist::ed::stdlike::atomic<uint32_t> value_{0};
};

}  // namespace stdlike
