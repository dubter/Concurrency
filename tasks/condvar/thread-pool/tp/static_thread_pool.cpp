#include <tp/static_thread_pool.hpp>

#include <tp/helpers.hpp>
#include <tp/thread_label.hpp>

#include <twist/util/thread_local.hpp>

namespace tp {

static twist::util::ThreadLocal<StaticThreadPool*> pool{nullptr};

////////////////////////////////////////////////////////////////////////////////

StaticThreadPool::StaticThreadPool(size_t /*workers*/, std::string /*name*/) {
  // Not implemented
}

StaticThreadPool::~StaticThreadPool() {
  // Not implemented
}

void StaticThreadPool::Submit(Task /*task*/) {
  // Not implemented
}

void StaticThreadPool::Join() {
  // Not implemented
}

void StaticThreadPool::Shutdown() {
  // Not implemented
}

StaticThreadPool* StaticThreadPool::Current() {
  return *pool;
}

}  // namespace tp
