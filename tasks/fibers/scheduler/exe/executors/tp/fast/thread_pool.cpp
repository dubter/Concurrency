#include <exe/executors/tp/fast/thread_pool.hpp>

#include <twist/util/thread_local.hpp>

namespace exe::executors::tp::fast {

ThreadPool::ThreadPool(size_t /*threads*/) {
  // Not implemented
}

ThreadPool::~ThreadPool() {
  // Not implemented
}

void ThreadPool::Execute(TaskBase* /*task*/) {
  // Not implemented
  // Use Worker::Current()
}

void ThreadPool::WaitIdle() {
  // Not implemented
}

void ThreadPool::Stop() {
  // Not implemented
}

PoolMetrics ThreadPool::Metrics() const {
  std::abort();  // Not implemented
}

ThreadPool* ThreadPool::Current() {
  return nullptr;  // Not implemented
  // Use Worker::Current()
}

}  // namespace exe::executors::tp::fast
