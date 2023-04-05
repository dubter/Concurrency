#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdint>

namespace stdlike {

class Mutex {
  enum State { Free = 0, Locked = 1, LockedWithContention = 2 };

 public:
  void Lock() {
    State free = State::Free;
    if (state_.compare_exchange_strong(free, State::Locked)) {
      return;
    }

    while (state_.exchange(State::LockedWithContention) != State::Free) {
      twist::ed::Wait(
          reinterpret_cast<twist::ed::stdlike::atomic<uint32_t>&>(state_),
          State::LockedWithContention);
    }
  }

  void Unlock() {
    auto key = twist::ed::PrepareWake(
        reinterpret_cast<twist::ed::stdlike::atomic<uint32_t>&>(state_));
    if (state_.exchange(State::Free) == State::LockedWithContention) {
      twist::ed::WakeOne(key);
    }
  }

  // BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::ed::stdlike::atomic<State> state_{State::Free};
};
}  // namespace stdlike
