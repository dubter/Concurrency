#pragma once

#include <futures/future.hpp>

#include <memory>

namespace stdlike {

template <typename T>
class Promise {
 public:
  Promise() {
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // One-shot
  Future<T> MakeFuture() {
    return Future<T>{state_};
  }

  // One-shot
  // Fulfill promise with value
  void SetValue(T /*value*/) {
    // Not implemented
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr /*ex*/) {
    // Not implemented
  }

 private:
  // ???
};

}  // namespace stdlike
