#pragma once

#include <gorr/runtime/join_handle.hpp>

namespace gorr {

inline void Join(JoinHandle /*handle*/) {
  std::abort();  // Not implemented
}

}  // namespace gorr
