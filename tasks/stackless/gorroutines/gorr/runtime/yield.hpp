#pragma once

#include <experimental/coroutine>

namespace gorr {

inline auto Yield() {
  // Not implemented
  return std::experimental::suspend_never{};
}

}  // namespace gorr
