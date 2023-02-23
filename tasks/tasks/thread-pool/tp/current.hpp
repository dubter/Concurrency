#pragma once

#include <tp/thread_pool.hpp>

namespace tp {

inline ThreadPool* Current() {
  return ThreadPool::Current();
}

}  // namespace tp
