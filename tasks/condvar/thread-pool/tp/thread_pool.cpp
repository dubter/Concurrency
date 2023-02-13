#include <tp/thread_pool.hpp>

#include <twist/ed/local/ptr.hpp>

namespace tp {

////////////////////////////////////////////////////////////////////////////////

static twist::ed::ThreadLocalPtr<ThreadPool> this_pool;

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t /*workers*/) {
  // Not implemented
}

ThreadPool::~ThreadPool() {
  // Not implemented
}

void ThreadPool::Submit(Task /*task*/) {
  // Not implemented
}

void ThreadPool::WaitIdle() {
  // Not implemented
}

void ThreadPool::Stop() {
  // Not implemented
}

ThreadPool* ThreadPool::Current() {
  return this_pool;
}

}  // namespace tp
