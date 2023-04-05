#include <exe/tp/thread_pool.hpp>

#include <twist/ed/local/ptr.hpp>
#include <wheels/core/panic.hpp>
#include <cassert>
#include <wheels/core/panic.hpp>

namespace exe::tp {

static twist::ed::ThreadLocalPtr<ThreadPool> thread_pool_ptr;

ThreadPool::ThreadPool(size_t num_threads) {
  num_threads_ = num_threads;
}

void ThreadPool::Work() {
  while (true) {
    std::optional<Task> task = blocking_queue_.Take();
    if (!(task.has_value())) {
      break;
    }
    task.value()();
    wait_group_.Done();
  }
}

void ThreadPool::Start() {
  for (size_t i = 0; i < num_threads_; ++i) {
    workers_.emplace_back([this] {
      thread_pool_ptr = this;
      Work();
    });
  }
}

ThreadPool::~ThreadPool() {
  assert(workers_.empty());
}

void ThreadPool::Submit(Task task) {
  wait_group_.Add(1);
  blocking_queue_.Put(std::move(task));
}

ThreadPool* ThreadPool::Current() {
  return thread_pool_ptr;
}

void ThreadPool::WaitIdle() {
  wait_group_.Wait();
}

void ThreadPool::Stop() {
  blocking_queue_.Close();
  for (auto& threads : workers_) {
    threads.join();
  }
  workers_.clear();
}

}  // namespace exe::tp
