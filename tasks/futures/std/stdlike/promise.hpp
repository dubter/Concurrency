#pragma once

#include <stdlike/future.hpp>

namespace stdlike {

template <typename T>
class Promise {
 public:
  Promise()
      : channel_(std::make_shared<detail::Channel<T>>()) {
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Movable
  Promise(Promise&&) = default;
  Promise& operator=(Promise&&) = default;

  // One-shot
  Future<T> MakeFuture() {
    return Future<T>(channel_);
  }

  // One-shot
  // Fulfill promise with value
  void SetValue(T value) {
    channel_->PutValue(std::move(value));
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr err) {
    channel_->PutException(std::move(err));
  }

 private:
  std::shared_ptr<detail::Channel<T>> channel_;
};

}  // namespace stdlike
