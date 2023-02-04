#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <optional>

namespace tp {

// Unbounded blocking multi-producers/multi-consumers queue

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T /*value*/) {
    return false;  // Not implemented
  }

  std::optional<T> Take() {
    return std::nullopt;  // Not implemented
  }

  void Close() {
    CloseImpl(/*clear=*/false);
  }

  void Cancel() {
    CloseImpl(/*clear=*/true);
  }

 private:
  void CloseImpl(bool /*clear*/) {
    // Not implemented
  }

 private:
  // Buffer
};

}  // namespace tp
