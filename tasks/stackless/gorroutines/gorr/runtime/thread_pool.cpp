#include <gorr/runtime/thread_pool.hpp>

namespace gorr {

StaticThreadPool::StaticThreadPool(size_t /*threads*/) {
  // Not implemented
}

StaticThreadPool::~StaticThreadPool() {
  // Not implemented
}

void StaticThreadPool::Join() {
  // Not implemented
}

StaticThreadPool* StaticThreadPool::Current() {
  return nullptr;  // Not implemented
}

}  // namespace gorr
