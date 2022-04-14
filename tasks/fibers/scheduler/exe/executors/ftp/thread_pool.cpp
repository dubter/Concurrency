#include <exe/executors/ftp/thread_pool.hpp>

#include <twist/util/thread_local.hpp>

namespace exe::executors::ftp {

FastThreadPool::FastThreadPool(size_t /*threads*/) {
  // Not implemented
}

FastThreadPool::~FastThreadPool() {
  // Not implemented
}

void FastThreadPool::Execute(TaskBase* /*task*/) {
  // Not implemented
  // Use Worker::Current()
}

void FastThreadPool::WaitIdle() {
  // Not implemented
}

void FastThreadPool::Stop() {
  // Not implemented
}

FastThreadPool* FastThreadPool::Current() {
  return nullptr;  // Not implemented
  // Use Worker::Current()
}

}  // namespace exe::executors::ftp
