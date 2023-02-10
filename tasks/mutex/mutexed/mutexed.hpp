#pragma once

#include <twist/ed/stdlike/mutex.hpp>

//////////////////////////////////////////////////////////////////////

/*
 * Safe API for mutual exclusion
 *
 * Usage:
 *
 * Mutexed<std::vector<Apple>> apples;
 *
 * {
 *   auto owner_ref = apples->Acquire();
 *   owner_ref->push_back(Apple{});
 * }  // <- release ownership
 *
 */

template <typename T>
class Mutexed {
  using Mutex = twist::ed::stdlike::mutex;

  class OwnerRef {
    // Your code goes here

    // Non-copyable
    OwnerRef(const OwnerRef&) = delete;
    OwnerRef& operator=(const OwnerRef&) = delete;

    // Non-movable
    OwnerRef(OwnerRef&&) = delete;
    OwnerRef& operator=(OwnerRef&) = delete;

    // operator*

    // operator->

   private:
    // ???
  };

 public:
  // https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c/
  template <typename... Args>
  explicit Mutexed(Args&&... args)
      : object_(std::forward<Args>(args)...) {
  }

  OwnerRef Acquire() {
    return {};
  }

 private:
  T object_;
  Mutex mutex_;  // Guards access to object_
};

//////////////////////////////////////////////////////////////////////

template <typename T>
auto Acquire(Mutexed<T>& object) {
  return object.Acquire();
}
