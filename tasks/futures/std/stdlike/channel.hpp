#pragma once

#include <cassert>
#include <variant>
#include <memory>

#include <twist/ed/stdlike/condition_variable.hpp>
#include <twist/ed/stdlike/mutex.hpp>

namespace stdlike::detail {

template <class T>
class Channel {
 public:
  Channel() = default;
  // Non-copyable
  Channel(const Channel&) = delete;
  Channel& operator=(const Channel&) = delete;

  void PutValue(T value) {
    std::lock_guard lock(mutex_);
    result_.template emplace<1>(std::move(value));
    is_empty_chan_.notify_all();
  }

  void PutException(std::exception_ptr err) {
    std::lock_guard lock(mutex_);
    result_.template emplace<2>(err);
    is_empty_chan_.notify_all();
  }

  T Get() {
    std::unique_lock lock(mutex_);
    while (result_.index() == 0) {
      is_empty_chan_.wait(lock);
    }

    if (result_.index() == 1) {
      return std::move(std::get<1>(result_));
    }
    std::rethrow_exception(std::get<2>(result_));
  }

 private:
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable is_empty_chan_;
  std::variant<std::monostate, T, std::exception_ptr> result_;
};
}  // namespace stdlike::detail
