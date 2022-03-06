#pragma once

#include <exe/coroutine/impl.hpp>
#include <context/stack.hpp>

#include <optional>

namespace exe::coroutine::processors {

template <typename T>
class Processor {
 public:
  explicit Processor(Routine /*routine*/) {
    // Not implemented
  }

  void Send(T /*value*/) {
    // Not implemented
  }

  void Close() {
    // Not implemented
  }

  static std::optional<T> Receive() {
    return std::nullopt;
  }

 private:
  static context::Stack AllocateStack() {
    // 16 * 4KB = 64KB
    return context::Stack::AllocatePages(16);
  }

 private:
  // ???
};

// Shortcut
template <typename T>
std::optional<T> Receive() {
  return Processor<T>::Receive();
}

}  // namespace exe::coroutine::processors
